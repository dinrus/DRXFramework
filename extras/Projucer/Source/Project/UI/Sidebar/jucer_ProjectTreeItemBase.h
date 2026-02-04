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
struct ProjectTreeItemBase : public JucerTreeViewBase,
                             public ValueTree::Listener
{
    ProjectTreeItemBase() {}

    z0 showSettingsPage (Component* content)
    {
        content->setComponentID (getUniqueName());
        std::unique_ptr<Component> comp (content);

        if (auto* pcc = getProjectContentComponent())
            pcc->setScrollableEditorComponent (std::move (comp));
    }

    z0 closeSettingsPage()
    {
        if (auto* pcc = getProjectContentComponent())
            if (auto* content = pcc->getEditorComponent())
                if (content->getComponentID() == getUniqueName())
                    pcc->hideEditor();
    }

    z0 deleteAllSelectedItems() override
    {
        auto* tree = getOwnerView();
        jassert (tree->getNumSelectedItems() <= 1); // multi-select should be disabled

        if (auto* s = dynamic_cast<ProjectTreeItemBase*> (tree->getSelectedItem (0)))
            s->deleteItem();
    }

    z0 itemOpennessChanged (b8 isNowOpen) override
    {
        if (isNowOpen)
           refreshSubItems();
    }

    virtual b8 isProjectSettings() const          { return false; }
    virtual b8 isModulesList() const              { return false; }

    static z0 updateSize (Component& comp, PropertyGroupComponent& group)
    {
        auto width = jmax (550, comp.getParentWidth() - 12);

        auto y = 0;
        y += group.updateSize (12, y, width - 12);

        y = jmax (comp.getParentHeight(), y);

        comp.setSize (width, y);
    }
};
