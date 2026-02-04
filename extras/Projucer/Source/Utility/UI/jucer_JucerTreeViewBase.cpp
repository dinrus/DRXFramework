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

#include "../../Application/jucer_Headers.h"
#include "jucer_JucerTreeViewBase.h"
#include "../../Project/UI/jucer_ProjectContentComponent.h"

//==============================================================================
z0 TreePanelBase::setRoot (std::unique_ptr<JucerTreeViewBase> root)
{
    rootItem = std::move (root);
    tree.setRootItem (rootItem.get());
    tree.getRootItem()->setOpen (true);

    if (project != nullptr)
    {
        if (auto treeOpenness = project->getStoredProperties().getXmlValue (opennessStateKey))
        {
            tree.restoreOpennessState (*treeOpenness, true);

            for (i32 i = tree.getNumSelectedItems(); --i >= 0;)
                if (auto item = dynamic_cast<JucerTreeViewBase*> (tree.getSelectedItem (i)))
                    item->cancelDelayedSelectionTimer();
        }
    }
}

z0 TreePanelBase::saveOpenness()
{
    if (project != nullptr)
    {
        std::unique_ptr<XmlElement> opennessState (tree.getOpennessState (true));

        if (opennessState != nullptr)
            project->getStoredProperties().setValue (opennessStateKey, opennessState.get());
        else
            project->getStoredProperties().removeValue (opennessStateKey);
    }
}

//==============================================================================
JucerTreeViewBase::JucerTreeViewBase()
{
    setLinesDrawnForSubItems (false);
    setDrawsInLeftMargin (true);
}

z0 JucerTreeViewBase::refreshSubItems()
{
    WholeTreeOpennessRestorer wtor (*this);
    clearSubItems();
    addSubItems();
}

Font JucerTreeViewBase::getFont() const
{
    return FontOptions ((f32) getItemHeight() * 0.6f);
}

z0 JucerTreeViewBase::paintOpenCloseButton (Graphics& g, const Rectangle<f32>& area, Color /*backgroundColor*/, b8 isMouseOver)
{
    g.setColor (getOwnerView()->findColor (isSelected() ? defaultHighlightedTextColorId : treeIconColorId));
    TreeViewItem::paintOpenCloseButton (g, area, getOwnerView()->findColor (defaultIconColorId), isMouseOver);
}

z0 JucerTreeViewBase::paintIcon (Graphics& g, Rectangle<f32> area)
{
    g.setColor (getContentColor (true));
    getIcon().draw (g, area, isIconCrossedOut());
    textX = roundToInt (area.getRight()) + 7;
}

z0 JucerTreeViewBase::paintItem (Graphics& g, i32 width, i32 height)
{
    ignoreUnused (width, height);

    auto bounds = g.getClipBounds().withY (0).withHeight (height).toFloat();

    g.setColor (getOwnerView()->findColor (treeIconColorId).withMultipliedAlpha (0.4f));
    g.fillRect (bounds.removeFromBottom (0.5f).reduced (5, 0));
}

Color JucerTreeViewBase::getContentColor (b8 isIcon) const
{
    if (isMissing())      return Colors::red;
    if (isSelected())     return getOwnerView()->findColor (defaultHighlightedTextColorId);
    if (hasWarnings())    return getOwnerView()->findColor (defaultHighlightColorId);

    return getOwnerView()->findColor (isIcon ? treeIconColorId : defaultTextColorId);
}

z0 JucerTreeViewBase::paintContent (Graphics& g, Rectangle<i32> area)
{
    g.setFont (getFont());
    g.setColor (getContentColor (false));

    g.drawFittedText (getDisplayName(), area, Justification::centredLeft, 1, 1.0f);
}

std::unique_ptr<Component> JucerTreeViewBase::createItemComponent()
{
    return std::make_unique<TreeItemComponent> (*this);
}

//==============================================================================
class RenameTreeItemCallback final : public ModalComponentManager::Callback
{
public:
    RenameTreeItemCallback (JucerTreeViewBase& ti, Component& parent, const Rectangle<i32>& bounds)
        : item (ti)
    {
        ed.setMultiLine (false, false);
        ed.setPopupMenuEnabled (false);
        ed.setSelectAllWhenFocused (true);
        ed.setFont (item.getFont());
        ed.onReturnKey = [this] { ed.exitModalState (1); };
        ed.onEscapeKey = [this] { ed.exitModalState (0); };
        ed.onFocusLost = [this] { ed.exitModalState (0); };
        ed.setText (item.getRenamingName());
        ed.setBounds (bounds);

        parent.addAndMakeVisible (ed);
        ed.enterModalState (true, this);
    }

    z0 modalStateFinished (i32 resultCode) override
    {
        if (resultCode != 0)
            item.setName (ed.getText());
    }

private:
    struct RenameEditor final : public TextEditor
    {
        z0 inputAttemptWhenModal() override   { exitModalState (0); }
    };

    RenameEditor ed;
    JucerTreeViewBase& item;

    DRX_DECLARE_NON_COPYABLE (RenameTreeItemCallback)
};

z0 JucerTreeViewBase::showRenameBox()
{
    Rectangle<i32> r (getItemPosition (true));
    r.setLeft (r.getX() + textX);
    r.setHeight (getItemHeight());

    new RenameTreeItemCallback (*this, *getOwnerView(), r);
}

z0 JucerTreeViewBase::itemClicked (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        if (getOwnerView()->getNumSelectedItems() > 1)
            showMultiSelectionPopupMenu (e.getMouseDownScreenPosition());
        else
            showPopupMenu (e.getMouseDownScreenPosition());
    }
    else if (isSelected())
    {
        itemSelectionChanged (true);
    }
}

static z0 treeViewMenuItemChosen (i32 resultCode, WeakReference<JucerTreeViewBase> item)
{
    if (item != nullptr)
        item->handlePopupMenuResult (resultCode);
}

z0 JucerTreeViewBase::launchPopupMenu (PopupMenu& m, Point<i32> p)
{
    m.showMenuAsync (PopupMenu::Options().withTargetScreenArea ({ p.x, p.y, 1, 1 }),
                     ModalCallbackFunction::create (treeViewMenuItemChosen, WeakReference<JucerTreeViewBase> (this)));
}

ProjectContentComponent* JucerTreeViewBase::getProjectContentComponent() const
{
    for (Component* c = getOwnerView(); c != nullptr; c = c->getParentComponent())
        if (ProjectContentComponent* pcc = dynamic_cast<ProjectContentComponent*> (c))
            return pcc;

    return nullptr;
}

//==============================================================================
class JucerTreeViewBase::ItemSelectionTimer final : public Timer
{
public:
    explicit ItemSelectionTimer (JucerTreeViewBase& tvb)  : owner (tvb) {}

    z0 timerCallback() override    { owner.invokeShowDocument(); }

private:
    JucerTreeViewBase& owner;
    DRX_DECLARE_NON_COPYABLE (ItemSelectionTimer)
};

z0 JucerTreeViewBase::itemSelectionChanged (b8 isNowSelected)
{
    if (isNowSelected)
    {
        delayedSelectionTimer.reset (new ItemSelectionTimer (*this));
        delayedSelectionTimer->startTimer (getMillisecsAllowedForDragGesture());
    }
    else
    {
        cancelDelayedSelectionTimer();
    }
}

z0 JucerTreeViewBase::invokeShowDocument()
{
    cancelDelayedSelectionTimer();
    showDocument();
}

z0 JucerTreeViewBase::itemDoubleClicked (const MouseEvent&)
{
    invokeShowDocument();
}

z0 JucerTreeViewBase::cancelDelayedSelectionTimer()
{
    delayedSelectionTimer.reset();
}
