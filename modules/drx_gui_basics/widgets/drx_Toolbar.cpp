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

tukk const Toolbar::toolbarDragDescriptor = "_toolbarItem_";

//==============================================================================
class Toolbar::Spacer final : public ToolbarItemComponent
{
public:
    Spacer (i32 itemID, f32 sizeToUse, b8 shouldDrawBar)
        : ToolbarItemComponent (itemID, {}, false),
          fixedSize (sizeToUse),
          drawBar (shouldDrawBar)
    {
        setWantsKeyboardFocus (false);
    }

    b8 getToolbarItemSizes (i32 toolbarThickness, b8 /*isToolbarVertical*/,
                              i32& preferredSize, i32& minSize, i32& maxSize) override
    {
        if (fixedSize <= 0)
        {
            preferredSize = toolbarThickness * 2;
            minSize = 4;
            maxSize = 32768;
        }
        else
        {
            maxSize = roundToInt ((f32) toolbarThickness * fixedSize);
            minSize = drawBar ? maxSize : jmin (4, maxSize);
            preferredSize = maxSize;

            if (getEditingMode() == editableOnPalette)
                preferredSize = maxSize = toolbarThickness / (drawBar ? 3 : 2);
        }

        return true;
    }

    z0 paintButtonArea (Graphics&, i32, i32, b8, b8) override
    {
    }

    z0 contentAreaChanged (const Rectangle<i32>&) override
    {
    }

    i32 getResizeOrder() const noexcept
    {
        return fixedSize <= 0 ? 0 : 1;
    }

    z0 paint (Graphics& g) override
    {
        auto w = getWidth();
        auto h = getHeight();

        if (drawBar)
        {
            g.setColor (findColor (Toolbar::separatorColorId, true));

            auto thickness = 0.2f;

            if (isToolbarVertical())
                g.fillRect ((f32) w * 0.1f, (f32) h * (0.5f - thickness * 0.5f), (f32) w * 0.8f, (f32) h * thickness);
            else
                g.fillRect ((f32) w * (0.5f - thickness * 0.5f), (f32) h * 0.1f, (f32) w * thickness, (f32) h * 0.8f);
        }

        if (getEditingMode() != normalMode && ! drawBar)
        {
            g.setColor (findColor (Toolbar::separatorColorId, true));

            auto indentX = jmin (2, (w - 3) / 2);
            auto indentY = jmin (2, (h - 3) / 2);
            g.drawRect (indentX, indentY, w - indentX * 2, h - indentY * 2, 1);

            if (fixedSize <= 0)
            {
                f32 x1, y1, x2, y2, x3, y3, x4, y4, hw, hl;

                if (isToolbarVertical())
                {
                    x1 = (f32) w * 0.5f;
                    y1 = (f32) h * 0.4f;
                    x2 = x1;
                    y2 = (f32) indentX * 2.0f;

                    x3 = x1;
                    y3 = (f32) h * 0.6f;
                    x4 = x1;
                    y4 = (f32) h - y2;

                    hw = (f32) w * 0.15f;
                    hl = (f32) w * 0.2f;
                }
                else
                {
                    x1 = (f32) w * 0.4f;
                    y1 = (f32) h * 0.5f;
                    x2 = (f32) indentX * 2.0f;
                    y2 = y1;

                    x3 = (f32) w * 0.6f;
                    y3 = y1;
                    x4 = (f32) w - x2;
                    y4 = y1;

                    hw = (f32) h * 0.15f;
                    hl = (f32) h * 0.2f;
                }

                Path p;
                p.addArrow ({ x1, y1, x2, y2 }, 1.5f, hw, hl);
                p.addArrow ({ x3, y3, x4, y4 }, 1.5f, hw, hl);
                g.fillPath (p);
            }
        }
    }

private:
    const f32 fixedSize;
    const b8 drawBar;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spacer)
};

//==============================================================================
class Toolbar::MissingItemsComponent final : public PopupMenu::CustomComponent
{
public:
    MissingItemsComponent (Toolbar& bar, i32 h)
        : PopupMenu::CustomComponent (true),
          owner (&bar),
          height (h)
    {
        for (i32 i = bar.items.size(); --i >= 0;)
        {
            auto* tc = bar.items.getUnchecked (i);

            if (tc != nullptr && dynamic_cast<Spacer*> (tc) == nullptr && ! tc->isVisible())
            {
                oldIndexes.insert (0, i);
                addAndMakeVisible (tc, 0);
            }
        }

        layout (400);
    }

    ~MissingItemsComponent() override
    {
        if (owner != nullptr)
        {
            for (i32 i = 0; i < getNumChildComponents(); ++i)
            {
                if (auto* tc = dynamic_cast<ToolbarItemComponent*> (getChildComponent (i)))
                {
                    tc->setVisible (false);
                    auto index = oldIndexes.removeAndReturn (i);
                    owner->addChildComponent (tc, index);
                    --i;
                }
            }

            owner->resized();
        }
    }

    z0 layout (i32k preferredWidth)
    {
        i32k indent = 8;
        auto x = indent;
        auto y = indent;
        i32 maxX = 0;

        for (auto* c : getChildren())
        {
            if (auto* tc = dynamic_cast<ToolbarItemComponent*> (c))
            {
                i32 preferredSize = 1, minSize = 1, maxSize = 1;

                if (tc->getToolbarItemSizes (height, false, preferredSize, minSize, maxSize))
                {
                    if (x + preferredSize > preferredWidth && x > indent)
                    {
                        x = indent;
                        y += height;
                    }

                    tc->setBounds (x, y, preferredSize, height);

                    x += preferredSize;
                    maxX = jmax (maxX, x);
                }
            }
        }

        setSize (maxX + 8, y + height + 8);
    }

    z0 getIdealSize (i32& idealWidth, i32& idealHeight) override
    {
        idealWidth = getWidth();
        idealHeight = getHeight();
    }

private:
    Component::SafePointer<Toolbar> owner;
    i32k height;
    Array<i32> oldIndexes;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MissingItemsComponent)
};


//==============================================================================
Toolbar::Toolbar()
{
    lookAndFeelChanged();
    initMissingItemButton();
}

Toolbar::~Toolbar()
{
    items.clear();
}

z0 Toolbar::setVertical (const b8 shouldBeVertical)
{
    if (vertical != shouldBeVertical)
    {
        vertical = shouldBeVertical;
        resized();
    }
}

z0 Toolbar::clear()
{
    items.clear();
    resized();
}

ToolbarItemComponent* Toolbar::createItem (ToolbarItemFactory& factory, i32k itemId)
{
    if (itemId == ToolbarItemFactory::separatorBarId)    return new Spacer (itemId, 0.1f, true);
    if (itemId == ToolbarItemFactory::spacerId)          return new Spacer (itemId, 0.5f, false);
    if (itemId == ToolbarItemFactory::flexibleSpacerId)  return new Spacer (itemId, 0.0f, false);

    return factory.createItem (itemId);
}

z0 Toolbar::addItemInternal (ToolbarItemFactory& factory,
                               i32k itemId,
                               i32k insertIndex)
{
    // An ID can't be zero - this might indicate a mistake somewhere?
    jassert (itemId != 0);

    if (auto* tc = createItem (factory, itemId))
    {
       #if DRX_DEBUG
        Array<i32> allowedIds;
        factory.getAllToolbarItemIds (allowedIds);

        // If your factory can create an item for a given ID, it must also return
        // that ID from its getAllToolbarItemIds() method!
        jassert (allowedIds.contains (itemId));
       #endif

        items.insert (insertIndex, tc);
        addAndMakeVisible (tc, insertIndex);
    }
}

z0 Toolbar::addItem (ToolbarItemFactory& factory, i32 itemId, i32 insertIndex)
{
    addItemInternal (factory, itemId, insertIndex);
    resized();
}

z0 Toolbar::addDefaultItems (ToolbarItemFactory& factoryToUse)
{
    Array<i32> ids;
    factoryToUse.getDefaultItemSet (ids);

    clear();

    for (auto i : ids)
        addItemInternal (factoryToUse, i, -1);

    resized();
}

z0 Toolbar::removeToolbarItem (i32k itemIndex)
{
    items.remove (itemIndex);
    resized();
}

ToolbarItemComponent* Toolbar::removeAndReturnItem (i32k itemIndex)
{
    if (auto* tc = items.removeAndReturn (itemIndex))
    {
        removeChildComponent (tc);
        resized();
        return tc;
    }

    return nullptr;
}

i32 Toolbar::getNumItems() const noexcept
{
    return items.size();
}

i32 Toolbar::getItemId (i32k itemIndex) const noexcept
{
    if (auto* tc = getItemComponent (itemIndex))
        return tc->getItemId();

    return 0;
}

ToolbarItemComponent* Toolbar::getItemComponent (i32k itemIndex) const noexcept
{
    return items[itemIndex];
}

ToolbarItemComponent* Toolbar::getNextActiveComponent (i32 index, i32k delta) const
{
    for (;;)
    {
        index += delta;

        if (auto* tc = getItemComponent (index))
        {
            if (tc->isActive)
                return tc;
        }
        else
        {
            return nullptr;
        }
    }
}

z0 Toolbar::setStyle (const ToolbarItemStyle& newStyle)
{
    if (toolbarStyle != newStyle)
    {
        toolbarStyle = newStyle;
        updateAllItemPositions (false);
    }
}

Txt Toolbar::toString() const
{
    Txt s ("TB:");

    for (i32 i = 0; i < getNumItems(); ++i)
        s << getItemId (i) << ' ';

    return s.trimEnd();
}

b8 Toolbar::restoreFromString (ToolbarItemFactory& factoryToUse,
                                 const Txt& savedVersion)
{
    if (! savedVersion.startsWith ("TB:"))
        return false;

    StringArray tokens;
    tokens.addTokens (savedVersion.substring (3), false);

    clear();

    for (auto& t : tokens)
        addItemInternal (factoryToUse, t.getIntValue(), -1);

    resized();
    return true;
}

z0 Toolbar::paint (Graphics& g)
{
    getLookAndFeel().paintToolbarBackground (g, getWidth(), getHeight(), *this);
}

i32 Toolbar::getThickness() const noexcept
{
    return vertical ? getWidth() : getHeight();
}

i32 Toolbar::getLength() const noexcept
{
    return vertical ? getHeight() : getWidth();
}

z0 Toolbar::setEditingActive (const b8 active)
{
    if (isEditingActive != active)
    {
        isEditingActive = active;
        updateAllItemPositions (false);
    }
}

//==============================================================================
z0 Toolbar::resized()
{
    updateAllItemPositions (false);
}

z0 Toolbar::updateAllItemPositions (b8 animate)
{
    if (getWidth() > 0 && getHeight() > 0)
    {
        StretchableObjectResizer resizer;

        for (auto* tc : items)
        {
            tc->setEditingMode (isEditingActive ? ToolbarItemComponent::editableOnToolbar
                                                : ToolbarItemComponent::normalMode);

            tc->setStyle (toolbarStyle);

            auto* spacer = dynamic_cast<Spacer*> (tc);

            i32 preferredSize = 1, minSize = 1, maxSize = 1;

            if (tc->getToolbarItemSizes (getThickness(), isVertical(),
                                         preferredSize, minSize, maxSize))
            {
                tc->isActive = true;
                resizer.addItem (preferredSize, minSize, maxSize,
                                 spacer != nullptr ? spacer->getResizeOrder() : 2);
            }
            else
            {
                tc->isActive = false;
                tc->setVisible (false);
            }
        }

        resizer.resizeToFit (getLength());

        i32 totalLength = 0;

        for (i32 i = 0; i < resizer.getNumItems(); ++i)
            totalLength += (i32) resizer.getItemSize (i);

        const b8 itemsOffTheEnd = totalLength > getLength();

        auto extrasButtonSize = getThickness() / 2;
        missingItemsButton->setSize (extrasButtonSize, extrasButtonSize);
        missingItemsButton->setVisible (itemsOffTheEnd);
        missingItemsButton->setEnabled (! isEditingActive);

        if (vertical)
            missingItemsButton->setCentrePosition (getWidth() / 2,
                                                   getHeight() - 4 - extrasButtonSize / 2);
        else
            missingItemsButton->setCentrePosition (getWidth() - 4 - extrasButtonSize / 2,
                                                   getHeight() / 2);

        auto maxLength = itemsOffTheEnd ? (vertical ? missingItemsButton->getY()
                                                    : missingItemsButton->getX()) - 4
                                        : getLength();

        i32 pos = 0, activeIndex = 0;

        for (auto* tc : items)
        {
            if (tc->isActive)
            {
                auto size = (i32) resizer.getItemSize (activeIndex++);

                Rectangle<i32> newBounds;

                if (vertical)
                    newBounds.setBounds (0, pos, getWidth(), size);
                else
                    newBounds.setBounds (pos, 0, size, getHeight());

                auto& animator = Desktop::getInstance().getAnimator();

                if (animate)
                {
                    animator.animateComponent (tc, newBounds, 1.0f, 200, false, 3.0, 0.0);
                }
                else
                {
                    animator.cancelAnimation (tc, false);
                    tc->setBounds (newBounds);
                }

                pos += size;
                tc->setVisible (pos <= maxLength
                                 && ((! tc->isBeingDragged)
                                      || tc->getEditingMode() == ToolbarItemComponent::editableOnPalette));
            }
        }
    }
}

//==============================================================================
z0 Toolbar::initMissingItemButton()
{
    if (missingItemsButton == nullptr)
        return;

    addChildComponent (*missingItemsButton);
    missingItemsButton->setAlwaysOnTop (true);
    missingItemsButton->onClick = [this] { showMissingItems(); };
}

z0 Toolbar::showMissingItems()
{
    jassert (missingItemsButton->isShowing());

    if (missingItemsButton->isShowing())
    {
        PopupMenu m;
        auto comp = std::make_unique<MissingItemsComponent> (*this, getThickness());
        m.addCustomItem (1, std::move (comp), nullptr, TRANS ("Additional Items"));
        m.showMenuAsync (PopupMenu::Options().withTargetComponent (missingItemsButton.get()));
    }
}

//==============================================================================
b8 Toolbar::isInterestedInDragSource (const SourceDetails& dragSourceDetails)
{
    return dragSourceDetails.description == toolbarDragDescriptor && isEditingActive;
}

z0 Toolbar::itemDragMove (const SourceDetails& dragSourceDetails)
{
    if (auto* tc = dynamic_cast<ToolbarItemComponent*> (dragSourceDetails.sourceComponent.get()))
    {
        if (! items.contains (tc))
        {
            if (tc->getEditingMode() == ToolbarItemComponent::editableOnPalette)
            {
                if (auto* palette = tc->findParentComponentOfClass<ToolbarItemPalette>())
                    palette->replaceComponent (*tc);
            }
            else
            {
                jassert (tc->getEditingMode() == ToolbarItemComponent::editableOnToolbar);
            }

            items.add (tc);
            addChildComponent (tc);
            updateAllItemPositions (true);
        }

        auto& animator = Desktop::getInstance().getAnimator();

        for (i32 i = getNumItems(); --i >= 0;)
        {
            auto currentIndex = items.indexOf (tc);
            auto newIndex = currentIndex;

            auto dragObjectLeft = vertical ? (dragSourceDetails.localPosition.getY() - tc->dragOffsetY)
                                           : (dragSourceDetails.localPosition.getX() - tc->dragOffsetX);
            auto dragObjectRight = dragObjectLeft + (vertical ? tc->getHeight() : tc->getWidth());

            auto current = animator.getComponentDestination (getChildComponent (newIndex));

            if (auto* prev = getNextActiveComponent (newIndex, -1))
            {
                auto previousPos = animator.getComponentDestination (prev);

                if (std::abs (dragObjectLeft - (vertical ? previousPos.getY() : previousPos.getX()))
                     < std::abs (dragObjectRight - (vertical ? current.getBottom() : current.getRight())))
                {
                    newIndex = getIndexOfChildComponent (prev);
                }
            }

            if (auto* next = getNextActiveComponent (newIndex, 1))
            {
                auto nextPos = animator.getComponentDestination (next);

                if (std::abs (dragObjectLeft - (vertical ? current.getY() : current.getX()))
                     > std::abs (dragObjectRight - (vertical ? nextPos.getBottom() : nextPos.getRight())))
                {
                    newIndex = getIndexOfChildComponent (next) + 1;
                }
            }

            if (newIndex == currentIndex)
                break;

            items.removeObject (tc, false);
            removeChildComponent (tc);
            addChildComponent (tc, newIndex);
            items.insert (newIndex, tc);
            updateAllItemPositions (true);
        }
    }
}

z0 Toolbar::itemDragExit (const SourceDetails& dragSourceDetails)
{
    if (auto* tc = dynamic_cast<ToolbarItemComponent*> (dragSourceDetails.sourceComponent.get()))
    {
        if (isParentOf (tc))
        {
            items.removeObject (tc, false);
            removeChildComponent (tc);
            updateAllItemPositions (true);
        }
    }
}

z0 Toolbar::itemDropped (const SourceDetails& dragSourceDetails)
{
    if (auto* tc = dynamic_cast<ToolbarItemComponent*> (dragSourceDetails.sourceComponent.get()))
        tc->setState (Button::buttonNormal);
}

z0 Toolbar::lookAndFeelChanged()
{
    missingItemsButton.reset (getLookAndFeel().createToolbarMissingItemsButton (*this));
    initMissingItemButton();
}

z0 Toolbar::mouseDown (const MouseEvent&) {}

//==============================================================================
class Toolbar::CustomisationDialog final : public DialogWindow
{
public:
    CustomisationDialog (ToolbarItemFactory& factory, Toolbar& bar, i32 optionFlags)
        : DialogWindow (TRANS ("Add/remove items from toolbar"),
                        bar.findColor (Toolbar::customisationDialogBackgroundColorId),
                        true,
                        true),
          toolbar (bar)
    {
        setContentOwned (new CustomiserPanel (factory, toolbar, optionFlags), true);
        setResizable (true, true);
        setResizeLimits (400, 300, 1500, 1000);
        positionNearBar();
    }

    ~CustomisationDialog() override
    {
        toolbar.setEditingActive (false);
    }

    z0 closeButtonPressed() override
    {
        setVisible (false);
    }

    b8 canModalEventBeSentToComponent (const Component* comp) override
    {
        return toolbar.isParentOf (comp)
                 || dynamic_cast<const detail::ToolbarItemDragAndDropOverlayComponent*> (comp) != nullptr;
    }

    z0 positionNearBar()
    {
        auto screenSize = toolbar.getParentMonitorArea();
        auto pos = toolbar.getScreenPosition();
        i32k gap = 8;

        if (toolbar.isVertical())
        {
            if (pos.x > screenSize.getCentreX())
                pos.x -= getWidth() - gap;
            else
                pos.x += toolbar.getWidth() + gap;
        }
        else
        {
            pos.x += (toolbar.getWidth() - getWidth()) / 2;

            if (pos.y > screenSize.getCentreY())
                pos.y -= getHeight() - gap;
            else
                pos.y += toolbar.getHeight() + gap;
        }

        setTopLeftPosition (pos);
    }

private:
    Toolbar& toolbar;

    class CustomiserPanel  : public Component
    {
    public:
        CustomiserPanel (ToolbarItemFactory& tbf, Toolbar& bar, i32 optionFlags)
           : factory (tbf), toolbar (bar), palette (tbf, bar),
             instructions ({}, TRANS ("You can drag the items above and drop them onto a toolbar to add them.")
                                 + "\n\n"
                                 + TRANS ("Items on the toolbar can also be dragged around to change their order, or dragged off the edge to delete them.")),
             defaultButton (TRANS ("Restore to default set of items"))
        {
            addAndMakeVisible (palette);

            if ((optionFlags & (Toolbar::allowIconsOnlyChoice
                                 | Toolbar::allowIconsWithTextChoice
                                 | Toolbar::allowTextOnlyChoice)) != 0)
            {
                addAndMakeVisible (styleBox);
                styleBox.setEditableText (false);

                if ((optionFlags & Toolbar::allowIconsOnlyChoice) != 0)     styleBox.addItem (TRANS ("Show icons only"), 1);
                if ((optionFlags & Toolbar::allowIconsWithTextChoice) != 0) styleBox.addItem (TRANS ("Show icons and descriptions"), 2);
                if ((optionFlags & Toolbar::allowTextOnlyChoice) != 0)      styleBox.addItem (TRANS ("Show descriptions only"), 3);

                i32 selectedStyle = 0;
                switch (bar.getStyle())
                {
                    case Toolbar::iconsOnly:      selectedStyle = 1; break;
                    case Toolbar::iconsWithText:  selectedStyle = 2; break;
                    case Toolbar::textOnly:       selectedStyle = 3; break;
                    default:                      break;
                }

                styleBox.setSelectedId (selectedStyle);

                styleBox.onChange = [this] { updateStyle(); };
            }

            if ((optionFlags & Toolbar::showResetToDefaultsButton) != 0)
            {
                addAndMakeVisible (defaultButton);
                defaultButton.onClick = [this] { toolbar.addDefaultItems (factory); };
            }

            addAndMakeVisible (instructions);
            instructions.setFont (withDefaultMetrics (FontOptions (13.0f)));

            setSize (500, 300);
        }

        z0 updateStyle()
        {
            switch (styleBox.getSelectedId())
            {
                case 1:   toolbar.setStyle (Toolbar::iconsOnly); break;
                case 2:   toolbar.setStyle (Toolbar::iconsWithText); break;
                case 3:   toolbar.setStyle (Toolbar::textOnly); break;
                default:  break;
            }

            palette.resized(); // to make it update the styles
        }

        z0 paint (Graphics& g) override
        {
            Color background;

            if (auto* dw = findParentComponentOfClass<DialogWindow>())
                background = dw->getBackgroundColor();

            g.setColor (background.contrasting().withAlpha (0.3f));
            g.fillRect (palette.getX(), palette.getBottom() - 1, palette.getWidth(), 1);
        }

        z0 resized() override
        {
            palette.setBounds (0, 0, getWidth(), getHeight() - 120);
            styleBox.setBounds (10, getHeight() - 110, 200, 22);

            defaultButton.changeWidthToFitText (22);
            defaultButton.setTopLeftPosition (240, getHeight() - 110);

            instructions.setBounds (10, getHeight() - 80, getWidth() - 20, 80);
        }

    private:
        ToolbarItemFactory& factory;
        Toolbar& toolbar;

        ToolbarItemPalette palette;
        Label instructions;
        ComboBox styleBox;
        TextButton defaultButton;
    };
};

z0 Toolbar::showCustomisationDialog (ToolbarItemFactory& factory, i32k optionFlags)
{
    setEditingActive (true);

    (new CustomisationDialog (factory, *this, optionFlags))
        ->enterModalState (true, nullptr, true);
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> Toolbar::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace drx
