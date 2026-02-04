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

#include "../../Utility/UI/PropertyComponents/jucer_LabelPropertyComponent.h"

//==============================================================================
struct ContentViewHeader final : public Component
{
    ContentViewHeader (Txt headerName, Icon headerIcon)
        : name (headerName), icon (headerIcon)
    {
        setTitle (name);
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (contentHeaderBackgroundColorId));

        auto bounds = getLocalBounds().reduced (20, 0);

        icon.withColor (Colors::white).draw (g, bounds.toFloat().removeFromRight (30), false);

        g.setColor (Colors::white);
        g.setFont (FontOptions (18.0f));
        g.drawFittedText (name, bounds, Justification::centredLeft, 1);
    }

    Txt name;
    Icon icon;
};

//==============================================================================
class ListBoxHeader final : public Component
{
public:
    ListBoxHeader (Array<Txt> columnHeaders)
    {
        for (auto s : columnHeaders)
        {
            addAndMakeVisible (headers.add (new Label (s, s)));
            widths.add (1.0f / (f32) columnHeaders.size());
        }

        setSize (200, 40);
    }

    ListBoxHeader (Array<Txt> columnHeaders, Array<f32> columnWidths)
    {
        jassert (columnHeaders.size() == columnWidths.size());

        for (const auto [index, s] : enumerate (columnHeaders))
        {
            addAndMakeVisible (headers.add (new Label (s, s)));
            widths.add (columnWidths.getUnchecked ((i32) index));
        }

        recalculateWidths();

        setSize (200, 40);
    }

    z0 resized() override
    {
        auto bounds = getLocalBounds();
        auto width = bounds.getWidth();

        auto index = 0;
        for (auto h : headers)
        {
            auto headerWidth = roundToInt ((f32) width * widths.getUnchecked (index));
            h->setBounds (bounds.removeFromLeft (headerWidth));
            ++index;
        }
    }

    z0 setColumnHeaderWidth (i32 index, f32 proportionOfWidth)
    {
        if (! (isPositiveAndBelow (index, headers.size()) && isPositiveAndNotGreaterThan (proportionOfWidth, 1.0f)))
        {
            jassertfalse;
            return;
        }

        widths.set (index, proportionOfWidth);
        recalculateWidths (index);
    }

    i32 getColumnX (i32 index)
    {
        auto prop = 0.0f;
        for (i32 i = 0; i < index; ++i)
            prop += widths.getUnchecked (i);

        return roundToInt (prop * (f32) getWidth());
    }

    f32 getProportionAtIndex (i32 index)
    {
        jassert (isPositiveAndBelow (index, widths.size()));
        return widths.getUnchecked (index);
    }

private:
    OwnedArray<Label> headers;
    Array<f32> widths;

    z0 recalculateWidths (i32 indexToIgnore = -1)
    {
        auto total = 0.0f;

        for (auto w : widths)
            total += w;

        if (approximatelyEqual (total, 1.0f))
            return;

        auto diff = 1.0f - total;
        auto amount = diff / static_cast<f32> (indexToIgnore == -1 ? widths.size() : widths.size() - 1);

        for (i32 i = 0; i < widths.size(); ++i)
        {
            if (i != indexToIgnore)
            {
                auto val = widths.getUnchecked (i);
                widths.set (i, val + amount);
            }
        }
    }
};

//==============================================================================
class InfoButton final : public Button
{
public:
    InfoButton (const Txt& infoToDisplay = {})
        : Button ({})
    {
        setTitle ("Info");

        if (infoToDisplay.isNotEmpty())
            setInfoToDisplay (infoToDisplay);

        setSize (20, 20);
    }

    z0 paintButton (Graphics& g, b8 isMouseOverButton, b8 isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (2);
        auto& icon = getIcons().info;

        g.setColor (findColor (treeIconColorId).withMultipliedAlpha (isMouseOverButton || isButtonDown ? 1.0f : 0.5f));

        if (isButtonDown)
            g.fillEllipse (bounds);
        else
            g.fillPath (icon, RectanglePlacement (RectanglePlacement::centred)
                        .getTransformToFit (icon.getBounds(), bounds));
    }

    z0 clicked() override
    {
        auto w = std::make_unique<InfoWindow> (info);
        w->setSize (width, w->getHeight() * numLines + 10);

        CallOutBox::launchAsynchronously (std::move (w), getScreenBounds(), nullptr);
    }

    using Button::clicked;

    z0 setInfoToDisplay (const Txt& infoToDisplay)
    {
        if (infoToDisplay.isNotEmpty())
        {
            info = infoToDisplay;

            auto stringWidth = roundToInt (GlyphArrangement::getStringWidth (FontOptions (14.0f), info));
            width = jmin (300, stringWidth);

            numLines += static_cast<i32> (stringWidth / width);

            setHelpText (info);
        }
    }

    z0 setAssociatedComponent (Component* comp)    { associatedComponent = comp; }
    Component* getAssociatedComponent()              { return associatedComponent; }

private:
    Txt info;
    Component* associatedComponent = nullptr;
    i32 width;
    i32 numLines = 1;

    //==============================================================================
    struct InfoWindow final : public Component
    {
        InfoWindow (const Txt& s)
            : stringToDisplay (s)
        {
            setSize (150, 14);
        }

        z0 paint (Graphics& g) override
        {
            g.fillAll (findColor (secondaryBackgroundColorId));

            g.setColor (findColor (defaultTextColorId));
            g.setFont (FontOptions (14.0f));
            g.drawFittedText (stringToDisplay, getLocalBounds(), Justification::centred, 15, 0.75f);
        }

        Txt stringToDisplay;
    };

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoButton)
};

//==============================================================================
class PropertyGroupComponent final : public Component,
                                     private TextPropertyComponent::Listener
{
public:
    PropertyGroupComponent (Txt name, Icon icon, Txt desc = {})
        : header (name, icon),
          description (desc)
    {
        addAndMakeVisible (header);
    }

    z0 setProperties (const PropertyListBuilder& newProps)
    {
        clearProperties();

        if (description.isNotEmpty())
            properties.push_back (std::make_unique<LabelPropertyComponent> (description, 16, FontOptions (16.0f),
                                                                            Justification::centredLeft));

        for (auto* comp : newProps.components)
            properties.push_back (std::unique_ptr<PropertyComponent> (comp));

        for (auto& prop : properties)
        {
            const auto propertyTooltip = prop->getTooltip();

            if (propertyTooltip.isNotEmpty())
            {
                // set the tooltip to empty so it only displays when its button is clicked
                prop->setTooltip ({});

                auto infoButton = std::make_unique<InfoButton> (propertyTooltip);
                infoButton->setAssociatedComponent (prop.get());

                auto propertyAndInfoWrapper = std::make_unique<PropertyAndInfoWrapper> (*prop, *infoButton.get());
                addAndMakeVisible (propertyAndInfoWrapper.get());
                propertyComponentsWithInfo.push_back (std::move (propertyAndInfoWrapper));

                infoButtons.push_back (std::move (infoButton));
            }
            else
            {
                addAndMakeVisible (prop.get());
            }

            if (auto* multiChoice = dynamic_cast<MultiChoicePropertyComponent*> (prop.get()))
                multiChoice->onHeightChange = [this] { updateSize(); };

            if (auto* text = dynamic_cast<TextPropertyComponent*> (prop.get()))
                if (text->isTextEditorMultiLine())
                    text->addListener (this);
        }
    }

    i32 updateSize (i32 x, i32 y, i32 width)
    {
        header.setBounds (0, 0, width, headerSize);
        auto height = header.getBottom() + 10;

        for (auto& pp : properties)
        {
            const auto propertyHeight = jmax (pp->getPreferredHeight(), getApproximateLabelHeight (*pp));

            auto iter = std::find_if (propertyComponentsWithInfo.begin(), propertyComponentsWithInfo.end(),
                                      [&pp] (const std::unique_ptr<PropertyAndInfoWrapper>& w) { return &w->propertyComponent == pp.get(); });

            if (iter != propertyComponentsWithInfo.end())
                (*iter)->setBounds (0, height, width - 10, propertyHeight);
            else
                pp->setBounds (40, height, width - 50, propertyHeight);

            if (shouldResizePropertyComponent (pp.get()))
                resizePropertyComponent (pp.get());

            height += pp->getHeight() + 10;
        }

        height += 16;

        setBounds (x, y, width, jmax (height, getParentHeight()));

        return height;
    }

    z0 paint (Graphics& g) override
    {
        g.fillAll (findColor (secondaryBackgroundColorId));
    }

    const std::vector<std::unique_ptr<PropertyComponent>>& getProperties() const noexcept
    {
        return properties;
    }

    z0 clearProperties()
    {
        propertyComponentsWithInfo.clear();
        infoButtons.clear();
        properties.clear();
    }

private:
    //==============================================================================
    struct PropertyAndInfoWrapper final : public Component
    {
        PropertyAndInfoWrapper (PropertyComponent& c, InfoButton& i)
            : propertyComponent (c),
              infoButton (i)
        {
            setFocusContainerType (FocusContainerType::focusContainer);
            setTitle (propertyComponent.getName());

            addAndMakeVisible (propertyComponent);
            addAndMakeVisible (infoButton);
        }

        z0 resized() override
        {
            auto bounds = getLocalBounds();

            bounds.removeFromLeft (40);
            bounds.removeFromRight (10);

            propertyComponent.setBounds (bounds);
            infoButton.setCentrePosition (20, bounds.getHeight() / 2);
        }

        PropertyComponent& propertyComponent;
        InfoButton& infoButton;
    };

    //==============================================================================
    z0 textPropertyComponentChanged (TextPropertyComponent* comp) override
    {
        auto fontHeight = [comp]
        {
            Label tmpLabel;
            return comp->getLookAndFeel().getLabelFont (tmpLabel).getHeight();
        }();

        auto lines = StringArray::fromLines (comp->getText());

        comp->setPreferredHeight (jmax (100, 10 + roundToInt (fontHeight * (f32) lines.size())));

        updateSize();
    }

    z0 updateSize()
    {
        updateSize (getX(), getY(), getWidth());

        if (auto* parent = getParentComponent())
            parent->parentSizeChanged();
    }

    b8 shouldResizePropertyComponent (PropertyComponent* p)
    {
        if (auto* textComp = dynamic_cast<TextPropertyComponent*> (p))
            return ! textComp->isTextEditorMultiLine();

        return (dynamic_cast<ChoicePropertyComponent*>  (p) != nullptr
             || dynamic_cast<ButtonPropertyComponent*>  (p) != nullptr
             || dynamic_cast<BooleanPropertyComponent*> (p) != nullptr);
    }

    z0 resizePropertyComponent (PropertyComponent* pp)
    {
        for (auto i = pp->getNumChildComponents() - 1; i >= 0; --i)
        {
            auto* child = pp->getChildComponent (i);

            auto bounds = child->getBounds();
            child->setBounds (bounds.withSizeKeepingCentre (child->getWidth(), pp->getPreferredHeight()));
        }
    }

    static i32 getApproximateLabelHeight (const PropertyComponent& pp)
    {
        auto availableTextWidth = ProjucerLookAndFeel::getTextWidthForPropertyComponent (pp);

        if (availableTextWidth == 0)
            return 0;

        const auto font = ProjucerLookAndFeel::getPropertyComponentFont();
        const auto labelWidth = GlyphArrangement::getStringWidth (font, pp.getName());
        const auto numLines = (i32) (labelWidth / (f32) availableTextWidth) + 1;
        return (i32) std::round ((f32) numLines * font.getHeight() * 1.1f);
    }

    //==============================================================================
    static constexpr i32 headerSize = 40;

    std::vector<std::unique_ptr<PropertyComponent>> properties;
    std::vector<std::unique_ptr<InfoButton>> infoButtons;
    std::vector<std::unique_ptr<PropertyAndInfoWrapper>> propertyComponentsWithInfo;

    ContentViewHeader header;
    Txt description;

    //==============================================================================
    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyGroupComponent)
};
