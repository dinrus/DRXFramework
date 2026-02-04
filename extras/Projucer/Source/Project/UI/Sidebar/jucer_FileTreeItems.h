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


//==============================================================================
class FileTreeItemBase : public JucerTreeViewBase,
                         private ValueTree::Listener
{
public:
    FileTreeItemBase (const Project::Item& projectItem)
        : item (projectItem), isFileMissing (false)
    {
        item.state.addListener (this);
    }

    ~FileTreeItemBase() override
    {
        item.state.removeListener (this);
    }

    //==============================================================================
    virtual b8 acceptsFileDrop (const StringArray& files) const = 0;
    virtual b8 acceptsDragItems (const OwnedArray<Project::Item>& selectedNodes) = 0;

    //==============================================================================
    Txt getDisplayName() const override              { return item.getName(); }
    Txt getRenamingName() const override             { return getDisplayName(); }
    z0 setName (const Txt& newName) override       { item.getNameValue() = newName; }
    b8 isMissing() const override                     { return isFileMissing; }
    virtual File getFile() const                        { return item.getFile(); }

    z0 deleteItem() override                          { item.removeItemFromProject(); }

    z0 deleteAllSelectedItems() override
    {
        auto* tree = getOwnerView();
        Array<File> filesToTrash;
        Array<Project::Item> itemsToRemove;

        for (i32 i = 0; i < tree->getNumSelectedItems(); ++i)
        {
            if (auto* p = dynamic_cast<FileTreeItemBase*> (tree->getSelectedItem (i)))
            {
                itemsToRemove.add (p->item);

                if (p->item.isGroup())
                {
                    for (i32 j = 0; j < p->item.getNumChildren(); ++j)
                    {
                        auto associatedFile = p->item.getChild (j).getFile();

                        if (associatedFile.existsAsFile())
                            filesToTrash.addIfNotAlreadyThere (associatedFile);
                    }
                }
                else if (p->getFile().existsAsFile())
                {
                    filesToTrash.addIfNotAlreadyThere (p->getFile());
                }
            }
        }

        WeakReference<FileTreeItemBase> treeRootItem { dynamic_cast<FileTreeItemBase*> (tree->getRootItem()) };

        if (treeRootItem == nullptr)
        {
            jassertfalse;
            return;
        }

        auto doDelete = [treeRootItem, itemsToRemove] (const Array<File>& fsToTrash)
        {
            if (treeRootItem == nullptr)
                return;

            auto& om = ProjucerApplication::getApp().openDocumentManager;

            for (auto i = fsToTrash.size(); --i >= 0;)
            {
                auto f = fsToTrash.getUnchecked (i);

                om.closeFileWithoutSaving (f);

                if (! f.moveToTrash())
                {
                    // xxx
                }
            }

            for (auto i = itemsToRemove.size(); --i >= 0;)
            {
                if (auto itemToRemove = treeRootItem->findTreeViewItem (itemsToRemove.getUnchecked (i)))
                {
                    if (auto* pcc = treeRootItem->getProjectContentComponent())
                    {
                        if (auto* fileInfoComp = dynamic_cast<FileGroupInformationComponent*> (pcc->getEditorComponent()))
                            if (fileInfoComp->getGroupPath() == itemToRemove->getFile().getFullPathName())
                                pcc->hideEditor();
                    }

                    om.closeFileWithoutSaving (itemToRemove->getFile());
                    itemToRemove->deleteItem();
                }
            }
        };

        if (! filesToTrash.isEmpty())
        {
            Txt fileList;
            auto maxFilesToList = 10;
            for (auto i = jmin (maxFilesToList, filesToTrash.size()); --i >= 0;)
                fileList << filesToTrash.getUnchecked (i).getFullPathName() << "\n";

            if (filesToTrash.size() > maxFilesToList)
                fileList << "\n...plus " << (filesToTrash.size() - maxFilesToList) << " more files...";

            auto options = MessageBoxOptions::makeOptionsYesNoCancel (MessageBoxIconType::NoIcon,
                                                                      "Delete Project Items",
                                                                      "As well as removing the selected item(s) from the project, do you also want to move their files to the trash:\n\n" + fileList,
                                                                      "Just remove references",
                                                                      "Also move files to Trash",
                                                                      "Cancel",
                                                                      tree->getTopLevelComponent());
            messageBox = AlertWindow::showScopedAsync (options, [treeRootItem, filesToTrash, doDelete] (i32 r) mutable
            {
                if (treeRootItem == nullptr)
                    return;

                if (r == 0)
                    return;

                if (r != 2)
                    filesToTrash.clear();

                doDelete (filesToTrash);
            });

            return;
        }

        doDelete (filesToTrash);
    }

    virtual z0 revealInFinder() const
    {
        getFile().revealToUser();
    }

    virtual z0 browseToAddExistingFiles()
    {
        auto location = item.isGroup() ? item.determineGroupFolder() : getFile();
        chooser = std::make_unique<FileChooser> ("Add Files to Jucer Project", location, "");
        auto flags = FileBrowserComponent::openMode
                   | FileBrowserComponent::canSelectFiles
                   | FileBrowserComponent::canSelectDirectories
                   | FileBrowserComponent::canSelectMultipleItems;

        chooser->launchAsync (flags, [this] (const FileChooser& fc)
        {
            if (fc.getResults().isEmpty())
                return;

            StringArray files;

            for (i32 i = 0; i < fc.getResults().size(); ++i)
                files.add (fc.getResults().getReference (i).getFullPathName());

            addFilesRetainingSortOrder (files);
        });
    }

    virtual z0 checkFileStatus()  // (recursive)
    {
        auto file = getFile();
        auto nowMissing = (file != File() && ! file.exists());

        if (nowMissing != isFileMissing)
        {
            isFileMissing = nowMissing;
            repaintItem();
        }
    }

    virtual z0 addFilesAtIndex (const StringArray& files, i32 insertIndex)
    {
        if (auto* p = getParentProjectItem())
            p->addFilesAtIndex (files, insertIndex);
    }

    virtual z0 addFilesRetainingSortOrder (const StringArray& files)
    {
        if (auto* p = getParentProjectItem())
            p->addFilesRetainingSortOrder (files);
    }

    virtual z0 moveSelectedItemsTo (OwnedArray<Project::Item>&, i32 /*insertIndex*/)
    {
        jassertfalse;
    }

    z0 showMultiSelectionPopupMenu (Point<i32> p) override
    {
        PopupMenu m;
        m.addItem (1, "Delete");

        m.showMenuAsync (PopupMenu::Options().withTargetScreenArea ({ p.x, p.y, 1, 1 }),
                         ModalCallbackFunction::create (treeViewMultiSelectItemChosen, this));
    }

    static z0 treeViewMultiSelectItemChosen (i32 resultCode, FileTreeItemBase* item)
    {
        switch (resultCode)
        {
            case 1:     item->deleteAllSelectedItems(); break;
            default:    break;
        }
    }

    virtual FileTreeItemBase* findTreeViewItem (const Project::Item& itemToFind)
    {
        if (item == itemToFind)
            return this;

        auto wasOpen = isOpen();
        setOpen (true);

        for (auto i = getNumSubItems(); --i >= 0;)
        {
            if (auto* pg = dynamic_cast<FileTreeItemBase*> (getSubItem (i)))
                if (auto* found = pg->findTreeViewItem (itemToFind))
                    return found;
        }

        setOpen (wasOpen);
        return nullptr;
    }

    //==============================================================================
    b8 mightContainSubItems() override                { return item.getNumChildren() > 0; }
    Txt getUniqueName() const override               { jassert (item.getID().isNotEmpty()); return item.getID(); }
    b8 canBeSelected() const override                 { return true; }
    Txt getTooltip() override                        { return {}; }
    File getDraggableFile() const override              { return getFile(); }

    var getDragSourceDescription() override
    {
        cancelDelayedSelectionTimer();
        return projectItemDragType;
    }

    z0 addSubItems() override
    {
        for (i32 i = 0; i < item.getNumChildren(); ++i)
            if (auto* p = createSubItem (item.getChild (i)))
                addSubItem (p);
    }

    z0 itemOpennessChanged (b8 isNowOpen) override
    {
        if (isNowOpen)
            refreshSubItems();
    }

    //==============================================================================
    b8 isInterestedInFileDrag (const StringArray& files) override
    {
        return acceptsFileDrop (files);
    }

    z0 filesDropped (const StringArray& files, i32 insertIndex) override
    {
        if (files.size() == 1 && File (files[0]).hasFileExtension (Project::projectFileExtension))
            ProjucerApplication::getApp().openFile (files[0], [] (b8) {});
        else
            addFilesAtIndex (files, insertIndex);
    }

    b8 isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override
    {
        OwnedArray<Project::Item> selectedNodes;
        getSelectedProjectItemsBeingDragged (dragSourceDetails, selectedNodes);

        return selectedNodes.size() > 0 && acceptsDragItems (selectedNodes);
    }

    z0 itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, i32 insertIndex) override
    {
        OwnedArray<Project::Item> selectedNodes;
        getSelectedProjectItemsBeingDragged (dragSourceDetails, selectedNodes);

        if (selectedNodes.size() > 0)
        {
            auto* tree = getOwnerView();
            std::unique_ptr<XmlElement> oldOpenness (tree->getOpennessState (false));

            moveSelectedItemsTo (selectedNodes, insertIndex);

            if (oldOpenness != nullptr)
                tree->restoreOpennessState (*oldOpenness, false);
        }
    }

    i32 getMillisecsAllowedForDragGesture() override
    {
        // for images, give the user longer to start dragging before assuming they're
        // clicking to select it for previewing..
        return item.isImageFile() ? 250 : JucerTreeViewBase::getMillisecsAllowedForDragGesture();
    }

    static z0 getSelectedProjectItemsBeingDragged (const DragAndDropTarget::SourceDetails& dragSourceDetails,
                                                     OwnedArray<Project::Item>& selectedNodes)
    {
        if (dragSourceDetails.description == projectItemDragType)
        {
            auto* tree = dynamic_cast<TreeView*> (dragSourceDetails.sourceComponent.get());

            if (tree == nullptr)
                tree = dragSourceDetails.sourceComponent->findParentComponentOfClass<TreeView>();

            if (tree != nullptr)
            {
                auto numSelected = tree->getNumSelectedItems();

                for (i32 i = 0; i < numSelected; ++i)
                    if (auto* p = dynamic_cast<FileTreeItemBase*> (tree->getSelectedItem (i)))
                        selectedNodes.add (new Project::Item (p->item));
            }
        }
    }

    FileTreeItemBase* getParentProjectItem() const
    {
        return dynamic_cast<FileTreeItemBase*> (getParentItem());
    }

    //==============================================================================
    Project::Item item;

protected:
    //==============================================================================
    z0 valueTreePropertyChanged (ValueTree& tree, const Identifier&) override
    {
        if (tree == item.state)
            repaintItem();
    }

    z0 valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { treeChildrenChanged (parentTree); }
    z0 valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, i32) override  { treeChildrenChanged (parentTree); }
    z0 valueTreeChildOrderChanged (ValueTree& parentTree, i32, i32) override    { treeChildrenChanged (parentTree); }

    b8 isFileMissing;

    virtual FileTreeItemBase* createSubItem (const Project::Item& node) = 0;

    Icon getIcon() const override
    {
        auto colour = getOwnerView()->findColor (isSelected() ? defaultHighlightedTextColorId
                                                               : treeIconColorId);

        return item.getIcon (isOpen()).withColor (colour);
    }

    b8 isIconCrossedOut() const override  { return item.isIconCrossedOut(); }

    z0 treeChildrenChanged (const ValueTree& parentTree)
    {
        if (parentTree == item.state)
        {
            refreshSubItems();
            treeHasChanged();
            setOpen (true);
        }
    }

    z0 triggerAsyncRename (const Project::Item& itemToRename)
    {
        struct RenameMessage final : public CallbackMessage
        {
            RenameMessage (TreeView* const t, const Project::Item& i)
                : tree (t), itemToRename (i)  {}

            z0 messageCallback() override
            {
                if (tree != nullptr)
                    if (auto* root = dynamic_cast<FileTreeItemBase*> (tree->getRootItem()))
                        if (auto* found = root->findTreeViewItem (itemToRename))
                            found->showRenameBox();
            }

        private:
            Component::SafePointer<TreeView> tree;
            Project::Item itemToRename;
        };

        (new RenameMessage (getOwnerView(), itemToRename))->post();
    }

    static z0 moveItems (OwnedArray<Project::Item>& selectedNodes, Project::Item destNode, i32 insertIndex)
    {
        for (auto i = selectedNodes.size(); --i >= 0;)
        {
            auto* n = selectedNodes.getUnchecked (i);

            if (destNode == *n || destNode.state.isAChildOf (n->state)) // Check for recursion.
                return;

            if (! destNode.canContain (*n))
                selectedNodes.remove (i);
        }

        // Don't include any nodes that are children of other selected nodes..
        for (auto i = selectedNodes.size(); --i >= 0;)
        {
            auto* n = selectedNodes.getUnchecked (i);

            for (auto j = selectedNodes.size(); --j >= 0;)
            {
                if (j != i && n->state.isAChildOf (selectedNodes.getUnchecked (j)->state))
                {
                    selectedNodes.remove (i);
                    break;
                }
            }
        }

        // Remove and re-insert them one at a time..
        for (i32 i = 0; i < selectedNodes.size(); ++i)
        {
            auto* selectedNode = selectedNodes.getUnchecked (i);

            if (selectedNode->state.getParent() == destNode.state
                  && indexOfNode (destNode.state, selectedNode->state) < insertIndex)
                --insertIndex;

            selectedNode->removeItemFromProject();
            destNode.addChild (*selectedNode, insertIndex++);
        }
    }

    static i32 indexOfNode (const ValueTree& parent, const ValueTree& child)
    {
        for (auto i = parent.getNumChildren(); --i >= 0;)
            if (parent.getChild (i) == child)
                return i;

        return -1;
    }

    ScopedMessageBox messageBox;

private:
    std::unique_ptr<FileChooser> chooser;

    DRX_DECLARE_WEAK_REFERENCEABLE (FileTreeItemBase)
};

//==============================================================================
class SourceFileItem final : public FileTreeItemBase
{
public:
    SourceFileItem (const Project::Item& projectItem)
        : FileTreeItemBase (projectItem)
    {
    }

    b8 acceptsFileDrop (const StringArray&) const override             { return false; }
    b8 acceptsDragItems (const OwnedArray<Project::Item>&) override    { return false; }

    Txt getDisplayName() const override
    {
        return getFile().getFileName();
    }

    z0 paintItem (Graphics& g, i32 width, i32 height) override
    {
        JucerTreeViewBase::paintItem (g, width, height);

        if (item.needsSaving())
        {
            auto bounds = g.getClipBounds().withY (0).withHeight (height);

            g.setFont (getFont());
            g.setColor (getContentColor (false));

            g.drawFittedText ("*", bounds.removeFromLeft (height), Justification::centred, 1);
        }
    }

    static File findCorrespondingHeaderOrCpp (const File& f)
    {
        if (f.hasFileExtension (sourceFileExtensions))  return f.withFileExtension (".h");
        if (f.hasFileExtension (headerFileExtensions))  return f.withFileExtension (".cpp");

        return {};
    }

    z0 setName (const Txt& newName) override
    {
        if (newName != File::createLegalFileName (newName))
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             "File Rename",
                                                             "That filename contained some illegal characters!");
            messageBox = AlertWindow::showScopedAsync (options, [this, item = item] (i32)
            {
                triggerAsyncRename (item);
            });
            return;
        }

        auto oldFile = getFile();
        auto newFile = oldFile.getSiblingFile (newName);
        auto correspondingFile = findCorrespondingHeaderOrCpp (oldFile);

        if (correspondingFile.exists() && newFile.hasFileExtension (oldFile.getFileExtension()))
        {
            auto correspondingItem = item.project.getMainGroup().findItemForFile (correspondingFile);

            if (correspondingItem.isValid())
            {
                auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::NoIcon,
                                                                       "File Rename",
                                                                       "Do you also want to rename the corresponding file \"" + correspondingFile.getFileName() + "\" to match?");
                messageBox = AlertWindow::showScopedAsync (options, [parent = WeakReference { this }, oldFile, newFile, correspondingFile, correspondingItem] (i32 result) mutable
                {
                    if (parent == nullptr || result == 0)
                        return;

                    if (! parent->item.renameFile (newFile))
                    {
                        auto opts = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                                      "File Rename",
                                                                      "Failed to rename \"" + oldFile.getFullPathName() + "\"!\n\nCheck your file permissions!");
                        parent->messageBox = AlertWindow::showScopedAsync (opts, nullptr);
                        return;
                    }

                    if (! correspondingItem.renameFile (newFile.withFileExtension (correspondingFile.getFileExtension())))
                    {
                        auto opts = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                                      "File Rename",
                                                                      "Failed to rename \"" + correspondingFile.getFullPathName() + "\"!\n\nCheck your file permissions!");
                        parent->messageBox = AlertWindow::showScopedAsync (opts, nullptr);
                    }
                });
            }
        }

        if (! item.renameFile (newFile))
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             "File Rename",
                                                             "Failed to rename the file!\n\nCheck your file permissions!");
            messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }
    }

    FileTreeItemBase* createSubItem (const Project::Item&) override
    {
        jassertfalse;
        return nullptr;
    }

    z0 showDocument() override
    {
        auto f = getFile();

        if (f.exists())
            if (auto* pcc = getProjectContentComponent())
                pcc->showEditorForFile (f, false);
    }

    z0 showPopupMenu (Point<i32> p) override
    {
        PopupMenu m;

        m.addItem (1, "Open in external editor");
        m.addItem (2,
                     #if DRX_MAC
                      "Reveal in Finder");
                     #else
                      "Reveal in Explorer");
                     #endif

        m.addItem (4, "Rename File...");
        m.addSeparator();

        m.addItem (5, "Binary Resource", true, item.shouldBeAddedToBinaryResources());
        m.addItem (6, "Xcode Resource", true, item.shouldBeAddedToXcodeResources());
        m.addItem (7, "Compile", item.isSourceFile(), item.shouldBeCompiled());
        m.addItem (8, "Skip PCH", item.shouldBeCompiled(), item.shouldSkipPCH());
        m.addSeparator();

        m.addItem (3, "Delete");

        launchPopupMenu (m, p);
    }

    z0 showAddMenu (Point<i32> p) override
    {
        if (auto* group = dynamic_cast<GroupItem*> (getParentItem()))
            group->showAddMenu (p);
    }

    z0 handlePopupMenuResult (i32 resultCode) override
    {
        switch (resultCode)
        {
            case 1:  getFile().startAsProcess(); break;
            case 2:  revealInFinder(); break;
            case 3:  deleteAllSelectedItems(); break;
            case 4:  triggerAsyncRename (item); break;
            case 5:  item.getShouldAddToBinaryResourcesValue().setValue (! item.shouldBeAddedToBinaryResources()); break;
            case 6:  item.getShouldAddToXcodeResourcesValue().setValue (! item.shouldBeAddedToXcodeResources()); break;
            case 7:  item.getShouldCompileValue().setValue (! item.shouldBeCompiled()); break;
            case 8:  item.getShouldSkipPCHValue().setValue (! item.shouldSkipPCH()); break;

            default:
                if (auto* parentGroup = dynamic_cast<GroupItem*> (getParentProjectItem()))
                    parentGroup->processCreateFileMenuItem (resultCode);

                break;
        }
    }

    DRX_DECLARE_WEAK_REFERENCEABLE (SourceFileItem)
};

//==============================================================================
class GroupItem final : public FileTreeItemBase
{
public:
    GroupItem (const Project::Item& projectItem, const Txt& filter = {})
        : FileTreeItemBase (projectItem),
          searchFilter (filter)
    {
    }

    b8 isRoot() const override                                 { return item.isMainGroup(); }
    b8 acceptsFileDrop (const StringArray&) const override     { return true; }

    z0 addNewGroup()
    {
        auto newGroup = item.addNewSubGroup ("New Group", 0);
        triggerAsyncRename (newGroup);
    }

    b8 acceptsDragItems (const OwnedArray<Project::Item>& selectedNodes) override
    {
        for (auto i = selectedNodes.size(); --i >= 0;)
            if (item.canContain (*selectedNodes.getUnchecked (i)))
                return true;

        return false;
    }

    z0 addFilesAtIndex (const StringArray& files, i32 insertIndex) override
    {
        for (auto f : files)
        {
            if (item.addFileAtIndex (f, insertIndex, true))
                ++insertIndex;
        }
    }

    z0 addFilesRetainingSortOrder (const StringArray& files) override
    {
        for (auto i = files.size(); --i >= 0;)
            item.addFileRetainingSortOrder (files[i], true);
    }

    z0 moveSelectedItemsTo (OwnedArray<Project::Item>& selectedNodes, i32 insertIndex) override
    {
        moveItems (selectedNodes, item, insertIndex);
    }

    z0 checkFileStatus() override
    {
        for (i32 i = 0; i < getNumSubItems(); ++i)
            if (auto* p = dynamic_cast<FileTreeItemBase*> (getSubItem (i)))
                p->checkFileStatus();
    }

    b8 isGroupEmpty (const Project::Item& group) // recursive
    {
        for (i32 i = 0; i < group.getNumChildren(); ++i)
        {
            auto child = group.getChild (i);

            if ((child.isGroup() && ! isGroupEmpty (child))
                   || (child.isFile() && child.getName().containsIgnoreCase (searchFilter)))
                return false;
        }

        return true;
    }

    FileTreeItemBase* createSubItem (const Project::Item& child) override
    {
        if (child.isGroup())
        {
            if (searchFilter.isNotEmpty() && isGroupEmpty (child))
                return nullptr;

            return new GroupItem (child, searchFilter);
        }

        if (child.isFile())
        {
            if (child.getName().containsIgnoreCase (searchFilter))
                return new SourceFileItem (child);

            return nullptr;
        }

        jassertfalse;
        return nullptr;
    }

    z0 showDocument() override
    {
        if (auto* pcc = getProjectContentComponent())
            pcc->setScrollableEditorComponent (std::make_unique<FileGroupInformationComponent> (item));
    }

    static z0 openAllGroups (TreeViewItem* root)
    {
        for (i32 i = 0; i < root->getNumSubItems(); ++i)
            if (auto* sub = root->getSubItem (i))
                openOrCloseAllSubGroups (*sub, true);
    }

    static z0 closeAllGroups (TreeViewItem* root)
    {
        for (i32 i = 0; i < root->getNumSubItems(); ++i)
            if (auto* sub = root->getSubItem (i))
                openOrCloseAllSubGroups (*sub, false);
    }

    static z0 openOrCloseAllSubGroups (TreeViewItem& treeItem, b8 shouldOpen)
    {
        treeItem.setOpen (shouldOpen);

        for (auto i = treeItem.getNumSubItems(); --i >= 0;)
            if (auto* sub = treeItem.getSubItem (i))
                openOrCloseAllSubGroups (*sub, shouldOpen);
    }

    static z0 setFilesToCompile (Project::Item projectItem, const b8 shouldCompile)
    {
        if (projectItem.isFile() && (projectItem.getFile().hasFileExtension (fileTypesToCompileByDefault)))
            projectItem.getShouldCompileValue() = shouldCompile;

        for (auto i = projectItem.getNumChildren(); --i >= 0;)
            setFilesToCompile (projectItem.getChild (i), shouldCompile);
    }

    z0 showPopupMenu (Point<i32> p) override
    {
        PopupMenu m;
        addCreateFileMenuItems (m);

        m.addSeparator();

        m.addItem (1, "Collapse all Groups");
        m.addItem (2, "Expand all Groups");

        if (! isRoot())
        {
            if (isOpen())
                m.addItem (3, "Collapse all Sub-groups");
            else
                m.addItem (4, "Expand all Sub-groups");
        }

        m.addSeparator();
        m.addItem (5, "Enable compiling of all enclosed files");
        m.addItem (6, "Disable compiling of all enclosed files");

        m.addSeparator();
        m.addItem (7, "Sort Items Alphabetically");
        m.addItem (8, "Sort Items Alphabetically (Groups first)");
        m.addSeparator();

        if (! isRoot())
        {
            m.addItem (9, "Rename...");
            m.addItem (10, "Delete");
        }

        launchPopupMenu (m, p);
    }

    z0 showAddMenu (Point<i32> p) override
    {
        PopupMenu m;
        addCreateFileMenuItems (m);

        launchPopupMenu (m, p);
    }

    z0 handlePopupMenuResult (i32 resultCode) override
    {
        switch (resultCode)
        {
            case 1:     closeAllGroups (getOwnerView()->getRootItem()); break;
            case 2:     openAllGroups (getOwnerView()->getRootItem()); break;
            case 3:     openOrCloseAllSubGroups (*this, false); break;
            case 4:     openOrCloseAllSubGroups (*this, true); break;
            case 5:     setFilesToCompile (item, true); break;
            case 6:     setFilesToCompile (item, false); break;
            case 7:     item.sortAlphabetically (false, false); break;
            case 8:     item.sortAlphabetically (true, false); break;
            case 9:     triggerAsyncRename (item); break;
            case 10:    deleteAllSelectedItems(); break;
            default:    processCreateFileMenuItem (resultCode); break;
        }
    }

    z0 addCreateFileMenuItems (PopupMenu& m)
    {
        m.addItem (1001, "Add New Group");
        m.addItem (1002, "Add Existing Files...");

        m.addSeparator();
        wizard.addWizardsToMenu (m);
    }

    z0 processCreateFileMenuItem (i32 menuID)
    {
        switch (menuID)
        {
            case 1001:  addNewGroup(); break;
            case 1002:  browseToAddExistingFiles(); break;

            default:
                jassert (getProject() != nullptr);
                wizard.runWizardFromMenu (menuID, *getProject(), item);
                break;
        }
    }

    Project* getProject()
    {
        if (auto* tv = getOwnerView())
            if (auto* pcc = tv->findParentComponentOfClass<ProjectContentComponent>())
                return pcc->getProject();

        return nullptr;
    }

    z0 setSearchFilter (const Txt& filter) override
    {
        searchFilter = filter;
        refreshSubItems();
    }

    Txt searchFilter;
    NewFileWizard wizard;
};
