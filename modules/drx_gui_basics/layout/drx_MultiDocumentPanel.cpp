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

MultiDocumentPanelWindow::MultiDocumentPanelWindow (Color backgroundColor)
    : DocumentWindow (Txt(), backgroundColor,
                      DocumentWindow::maximiseButton | DocumentWindow::closeButton, false)
{
}

MultiDocumentPanelWindow::~MultiDocumentPanelWindow()
{
}

//==============================================================================
z0 MultiDocumentPanelWindow::maximiseButtonPressed()
{
    if (auto* owner = getOwner())
        owner->setLayoutMode (MultiDocumentPanel::MaximisedWindowsWithTabs);
    else
        jassertfalse; // these windows are only designed to be used inside a MultiDocumentPanel!
}

z0 MultiDocumentPanelWindow::closeButtonPressed()
{
    if (auto* owner = getOwner())
        owner->closeDocumentAsync (getContentComponent(), true, nullptr);
    else
        jassertfalse; // these windows are only designed to be used inside a MultiDocumentPanel!
}

z0 MultiDocumentPanelWindow::activeWindowStatusChanged()
{
    DocumentWindow::activeWindowStatusChanged();
    updateActiveDocument();
}

z0 MultiDocumentPanelWindow::broughtToFront()
{
    DocumentWindow::broughtToFront();
    updateActiveDocument();
}

z0 MultiDocumentPanelWindow::updateActiveDocument()
{
    if (auto* owner = getOwner())
        owner->updateActiveDocumentFromUIState();
}

MultiDocumentPanel* MultiDocumentPanelWindow::getOwner() const noexcept
{
    return findParentComponentOfClass<MultiDocumentPanel>();
}

//==============================================================================
struct MultiDocumentPanel::TabbedComponentInternal   : public TabbedComponent
{
    TabbedComponentInternal() : TabbedComponent (TabbedButtonBar::TabsAtTop) {}

    z0 currentTabChanged (i32, const Txt&) override
    {
        if (auto* owner = findParentComponentOfClass<MultiDocumentPanel>())
            owner->updateActiveDocumentFromUIState();
    }
};


//==============================================================================
MultiDocumentPanel::MultiDocumentPanel()
{
    setOpaque (true);
}

MultiDocumentPanel::~MultiDocumentPanel()
{
    for (i32 i = components.size(); --i >= 0;)
        if (auto* component = components[i])
            closeDocumentInternal (component);
}

//==============================================================================
namespace MultiDocHelpers
{
    static b8 shouldDeleteComp (Component* const c)
    {
        return c->getProperties() ["mdiDocumentDelete_"];
    }
}

#if DRX_MODAL_LOOPS_PERMITTED
b8 MultiDocumentPanel::closeAllDocuments (const b8 checkItsOkToCloseFirst)
{
    while (! components.isEmpty())
        if (! closeDocument (components.getLast(), checkItsOkToCloseFirst))
            return false;

    return true;
}
#endif

z0 MultiDocumentPanel::closeLastDocumentRecursive (SafePointer<MultiDocumentPanel> parent,
                                                     b8 checkItsOkToCloseFirst,
                                                     std::function<z0 (b8)> callback)
{
    if (parent->components.isEmpty())
    {
        NullCheckedInvocation::invoke (callback, true);
        return;
    }

    parent->closeDocumentAsync (parent->components.getLast(),
                                checkItsOkToCloseFirst,
                                [parent, checkItsOkToCloseFirst, callback] (b8 closeResult)
    {
        if (parent == nullptr)
            return;

        if (! closeResult)
        {
            NullCheckedInvocation::invoke (callback, false);
            return;
        }

        parent->closeLastDocumentRecursive (parent, checkItsOkToCloseFirst, std::move (callback));
    });
}

z0 MultiDocumentPanel::closeAllDocumentsAsync (b8 checkItsOkToCloseFirst, std::function<z0 (b8)> callback)
{
    closeLastDocumentRecursive (this, checkItsOkToCloseFirst, std::move (callback));
}

#if DRX_MODAL_LOOPS_PERMITTED
b8 MultiDocumentPanel::tryToCloseDocument (Component*)
{
    // If you hit this assertion then you need to implement this method in a subclass.
    jassertfalse;
    return false;
}
#endif

MultiDocumentPanelWindow* MultiDocumentPanel::createNewDocumentWindow()
{
    return new MultiDocumentPanelWindow (backgroundColor);
}

z0 MultiDocumentPanel::addWindow (Component* component)
{
    auto* dw = createNewDocumentWindow();

    dw->setResizable (true, false);
    dw->setContentNonOwned (component, true);
    dw->setName (component->getName());

    auto bkg = component->getProperties() ["mdiDocumentBkg_"];
    dw->setBackgroundColor (bkg.isVoid() ? backgroundColor : Color ((u32) static_cast<i32> (bkg)));

    i32 x = 4;

    if (auto* topComp = getChildren().getLast())
        if (topComp->getX() == x && topComp->getY() == x)
            x += 16;

    dw->setTopLeftPosition (x, x);

    auto pos = component->getProperties() ["mdiDocumentPos_"];
    if (pos.toString().isNotEmpty())
        dw->restoreWindowStateFromString (pos.toString());

    addAndMakeVisible (dw);
    dw->toFront (true);
}

b8 MultiDocumentPanel::addDocument (Component* const component,
                                      Color docColor,
                                      const b8 deleteWhenRemoved)
{
    // If you try passing a full DocumentWindow or ResizableWindow in here, you'll end up
    // with a frame-within-a-frame! Just pass in the bare content component.
    jassert (dynamic_cast<ResizableWindow*> (component) == nullptr);

    if (component == nullptr || (maximumNumDocuments > 0 && components.size() >= maximumNumDocuments))
        return false;

    components.add (component);
    component->getProperties().set ("mdiDocumentDelete_", deleteWhenRemoved);
    component->getProperties().set ("mdiDocumentBkg_", (i32) docColor.getARGB());
    component->addComponentListener (this);

    if (mode == FloatingWindows)
    {
        if (isFullscreenWhenOneDocument())
        {
            if (components.size() == 1)
            {
                addAndMakeVisible (component);
            }
            else
            {
                if (components.size() == 2)
                    addWindow (components.getFirst());

                addWindow (component);
            }
        }
        else
        {
           addWindow (component);
        }
    }
    else
    {
        if (tabComponent == nullptr && components.size() > numDocsBeforeTabsUsed)
        {
            tabComponent.reset (new TabbedComponentInternal());
            addAndMakeVisible (tabComponent.get());

            auto temp = components;

            for (auto& c : temp)
                tabComponent->addTab (c->getName(), docColor, c, false);

            resized();
        }
        else
        {
            if (tabComponent != nullptr)
                tabComponent->addTab (component->getName(), docColor, component, false);
            else
                addAndMakeVisible (component);
        }

        setActiveDocument (component);
    }

    resized();
    updateActiveDocument (component);
    return true;
}

z0 MultiDocumentPanel::recreateLayout()
{
    tabComponent.reset();

    for (i32 i = getNumChildComponents(); --i >= 0;)
    {
        std::unique_ptr<MultiDocumentPanelWindow> dw (dynamic_cast<MultiDocumentPanelWindow*> (getChildComponent (i)));

        if (dw != nullptr)
        {
            dw->getContentComponent()->getProperties().set ("mdiDocumentPos_", dw->getWindowStateAsString());
            dw->clearContentComponent();
        }
    }

    resized();

    auto tempComps = components;
    components.clear();

    {
        // We want to preserve the activeComponent, so we are blocking the changes originating
        // from addDocument()
        const ScopedValueSetter<b8> scope { isLayoutBeingChanged, true };

        for (auto* c : tempComps)
            addDocument (c,
                         Color ((u32) static_cast<i32> (c->getProperties().getWithDefault ("mdiDocumentBkg_",
                                                                                               (i32) Colors::white.getARGB()))),
                         MultiDocHelpers::shouldDeleteComp (c));
    }

    if (activeComponent != nullptr)
        setActiveDocument (activeComponent);

    updateActiveDocumentFromUIState();
}

z0 MultiDocumentPanel::closeDocumentInternal (Component* componentToClose)
{
    // Intellisense warns about component being uninitialised.
    // I'm not sure how a function argument could be uninitialised.
    DRX_BEGIN_IGNORE_WARNINGS_MSVC (6001)

    const OptionalScopedPointer<Component> component { componentToClose,
                                                       MultiDocHelpers::shouldDeleteComp (componentToClose) };

    component->removeComponentListener (this);

    component->getProperties().remove ("mdiDocumentDelete_");
    component->getProperties().remove ("mdiDocumentBkg_");

    const auto removedIndex = components.indexOf (component);

    if (removedIndex < 0)
    {
        jassertfalse;
        return;
    }

    components.remove (removedIndex);

    // See if the active document needs to change because of closing a document. It should only
    // change if we closed the active document. If so, the next active document should be the
    // subsequent one.
    if (component == activeComponent)
    {
        auto* newActiveComponent = components[jmin (removedIndex, components.size() - 1)];
        updateActiveDocument (newActiveComponent);
    }

    // We update the UI to reflect the new state, but we want to prevent the UI state callback
    // to change the active document.
    const ScopedValueSetter<b8> scope { isLayoutBeingChanged, true };

    if (mode == FloatingWindows)
    {
        for (auto* child : getChildren())
        {
            if (auto* dw = dynamic_cast<MultiDocumentPanelWindow*> (child))
            {
                if (dw->getContentComponent() == component)
                {
                    std::unique_ptr<MultiDocumentPanelWindow> (dw)->clearContentComponent();
                    break;
                }
            }
        }

        if (isFullscreenWhenOneDocument() && components.size() == 1)
        {
            for (i32 i = getNumChildComponents(); --i >= 0;)
            {
                std::unique_ptr<MultiDocumentPanelWindow> dw (dynamic_cast<MultiDocumentPanelWindow*> (getChildComponent (i)));

                if (dw != nullptr)
                    dw->clearContentComponent();
            }

            addAndMakeVisible (getActiveDocument());
        }
    }
    else
    {
        if (tabComponent != nullptr)
        {
            for (i32 i = tabComponent->getNumTabs(); --i >= 0;)
                if (tabComponent->getTabContentComponent (i) == component)
                    tabComponent->removeTab (i);
        }
        else
        {
            removeChildComponent (component);
        }

        if (components.size() <= numDocsBeforeTabsUsed && getActiveDocument() != nullptr)
        {
            tabComponent.reset();
            addAndMakeVisible (getActiveDocument());
        }
    }

    resized();

    // This ensures that the active tab is painted properly when a tab is closed!
    if (auto* activeDocument = getActiveDocument())
        setActiveDocument (activeDocument);

    DRX_END_IGNORE_WARNINGS_MSVC
}

#if DRX_MODAL_LOOPS_PERMITTED
b8 MultiDocumentPanel::closeDocument (Component* component,
                                        const b8 checkItsOkToCloseFirst)
{
    // Intellisense warns about component being uninitialised.
    // I'm not sure how a function argument could be uninitialised.
    DRX_BEGIN_IGNORE_WARNINGS_MSVC (6001)

    if (component == nullptr)
        return true;

    if (components.contains (component))
    {
        if (checkItsOkToCloseFirst && ! tryToCloseDocument (component))
            return false;

        closeDocumentInternal (component);
    }
    else
    {
        jassertfalse;
    }

    return true;

    DRX_END_IGNORE_WARNINGS_MSVC
}
#endif

z0 MultiDocumentPanel::closeDocumentAsync (Component* component,
                                             const b8 checkItsOkToCloseFirst,
                                             std::function<z0 (b8)> callback)
{
    // Intellisense warns about component being uninitialised.
    // I'm not sure how a function argument could be uninitialised.
    DRX_BEGIN_IGNORE_WARNINGS_MSVC (6001)

    if (component == nullptr)
    {
        NullCheckedInvocation::invoke (callback, true);
        return;
    }

    if (components.contains (component))
    {
        if (checkItsOkToCloseFirst)
        {
            tryToCloseDocumentAsync (component,
                                     [parent = SafePointer<MultiDocumentPanel> { this }, component, callback] (b8 closedSuccessfully)
            {
                if (parent == nullptr)
                    return;

                if (closedSuccessfully)
                    parent->closeDocumentInternal (component);

                NullCheckedInvocation::invoke (callback, closedSuccessfully);
            });

            return;
        }

        closeDocumentInternal (component);
    }
    else
    {
        jassertfalse;
    }

    NullCheckedInvocation::invoke (callback, true);

    DRX_END_IGNORE_WARNINGS_MSVC
}

i32 MultiDocumentPanel::getNumDocuments() const noexcept
{
    return components.size();
}

Component* MultiDocumentPanel::getDocument (i32k index) const noexcept
{
    return components [index];
}

Component* MultiDocumentPanel::getActiveDocument() const noexcept
{
    return activeComponent;
}

z0 MultiDocumentPanel::setActiveDocument (Component* component)
{
    jassert (component != nullptr);

    if (mode == FloatingWindows)
    {
        component = getContainerComp (component);

        if (component != nullptr)
            component->toFront (true);
    }
    else if (tabComponent != nullptr)
    {
        jassert (components.indexOf (component) >= 0);

        for (i32 i = tabComponent->getNumTabs(); --i >= 0;)
        {
            if (tabComponent->getTabContentComponent (i) == component)
            {
                tabComponent->setCurrentTabIndex (i);
                break;
            }
        }
    }
    else
    {
        component->grabKeyboardFocus();
    }
}

z0 MultiDocumentPanel::activeDocumentChanged()
{
}

z0 MultiDocumentPanel::setMaximumNumDocuments (i32k newNumber)
{
    maximumNumDocuments = newNumber;
}

z0 MultiDocumentPanel::useFullscreenWhenOneDocument (const b8 shouldUseTabs)
{
    const auto newNumDocsBeforeTabsUsed = shouldUseTabs ? 1 : 0;

    if (std::exchange (numDocsBeforeTabsUsed, newNumDocsBeforeTabsUsed) != newNumDocsBeforeTabsUsed)
        recreateLayout();
}

b8 MultiDocumentPanel::isFullscreenWhenOneDocument() const noexcept
{
    return numDocsBeforeTabsUsed != 0;
}

//==============================================================================
z0 MultiDocumentPanel::setLayoutMode (const LayoutMode newLayoutMode)
{
    if (std::exchange (mode, newLayoutMode) != newLayoutMode)
        recreateLayout();
}

z0 MultiDocumentPanel::setBackgroundColor (Color newBackgroundColor)
{
    if (backgroundColor != newBackgroundColor)
    {
        backgroundColor = newBackgroundColor;
        setOpaque (newBackgroundColor.isOpaque());
        repaint();
    }
}

//==============================================================================
z0 MultiDocumentPanel::paint (Graphics& g)
{
    g.fillAll (backgroundColor);
}

z0 MultiDocumentPanel::resized()
{
    if (mode == MaximisedWindowsWithTabs || components.size() == numDocsBeforeTabsUsed)
    {
        for (auto* child : getChildren())
            child->setBounds (getLocalBounds());
    }

    setWantsKeyboardFocus (components.size() == 0);
}

Component* MultiDocumentPanel::getContainerComp (Component* c) const
{
    if (mode == FloatingWindows)
    {
        for (auto* child : getChildren())
            if (auto* dw = dynamic_cast<MultiDocumentPanelWindow*> (child))
                if (dw->getContentComponent() == c)
                    return dw;
    }

    return c;
}

z0 MultiDocumentPanel::componentNameChanged (Component&)
{
    if (mode == FloatingWindows)
    {
        for (auto* child : getChildren())
            if (auto* dw = dynamic_cast<MultiDocumentPanelWindow*> (child))
                dw->setName (dw->getContentComponent()->getName());
    }
    else if (tabComponent != nullptr)
    {
        for (i32 i = tabComponent->getNumTabs(); --i >= 0;)
            tabComponent->setTabName (i, tabComponent->getTabContentComponent (i)->getName());
    }
}

z0 MultiDocumentPanel::updateActiveDocumentFromUIState()
{
    auto* newActiveComponent = [&]() -> Component*
    {
        if (mode == FloatingWindows)
        {
            for (auto* c : components)
            {
                if (auto* window = static_cast<MultiDocumentPanelWindow*> (c->getParentComponent()))
                    if (window->isActiveWindow())
                        return c;
            }
        }

        if (tabComponent != nullptr)
            if (auto* current = tabComponent->getCurrentContentComponent())
                return current;

        return activeComponent;
    }();

    updateActiveDocument (newActiveComponent);
}

z0 MultiDocumentPanel::updateActiveDocument (Component* component)
{
    if (isLayoutBeingChanged)
        return;

    if (std::exchange (activeComponent, component) != component)
        activeDocumentChanged();
}

} // namespace drx
