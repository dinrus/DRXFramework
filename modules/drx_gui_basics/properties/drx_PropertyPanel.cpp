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

struct PropertyPanel::SectionComponent final : public Component
{
    SectionComponent (const Txt& sectionTitle,
                      const Array<PropertyComponent*>& newProperties,
                      b8 sectionIsOpen,
                      i32 extraPadding)
        : Component (sectionTitle),
          isOpen (sectionIsOpen),
          padding (extraPadding)
    {
        lookAndFeelChanged();

        propertyComps.addArray (newProperties);

        for (auto* propertyComponent : propertyComps)
        {
            addAndMakeVisible (propertyComponent);
            propertyComponent->refresh();
        }
    }

    ~SectionComponent() override
    {
        propertyComps.clear();
    }

    z0 paint (Graphics& g) override
    {
        if (titleHeight > 0)
            getLookAndFeel().drawPropertyPanelSectionHeader (g, getName(), isOpen, getWidth(), titleHeight);
    }

    z0 resized() override
    {
        auto y = titleHeight;

        for (auto* propertyComponent : propertyComps)
        {
            propertyComponent->setBounds (1, y, getWidth() - 2, propertyComponent->getPreferredHeight());
            y = propertyComponent->getBottom() + padding;
        }
    }

    z0 lookAndFeelChanged() override
    {
        titleHeight = getLookAndFeel().getPropertyPanelSectionHeaderHeight (getName());
        resized();
        repaint();
    }

    i32 getPreferredHeight() const
    {
        auto y = titleHeight;

        auto numComponents = propertyComps.size();

        if (numComponents > 0 && isOpen)
        {
            for (auto* propertyComponent : propertyComps)
                y += propertyComponent->getPreferredHeight();

            y += (numComponents - 1) * padding;
        }

        return y;
    }

    z0 setOpen (b8 open)
    {
        if (isOpen != open)
        {
            isOpen = open;

            for (auto* propertyComponent : propertyComps)
                propertyComponent->setVisible (open);

            if (auto* propertyPanel = findParentComponentOfClass<PropertyPanel>())
                propertyPanel->resized();
        }
    }

    z0 refreshAll() const
    {
        for (auto* propertyComponent : propertyComps)
            propertyComponent->refresh();
    }

    z0 mouseUp (const MouseEvent& e) override
    {
        if (e.getMouseDownX() < titleHeight
              && e.x < titleHeight
              && e.getNumberOfClicks() != 2)
            mouseDoubleClick (e);
    }

    z0 mouseDoubleClick (const MouseEvent& e) override
    {
        if (e.y < titleHeight)
            setOpen (! isOpen);
    }

    OwnedArray<PropertyComponent> propertyComps;
    i32 titleHeight;
    b8 isOpen;
    i32 padding;

    DRX_DECLARE_NON_COPYABLE (SectionComponent)
};

//==============================================================================
struct PropertyPanel::PropertyHolderComponent final : public Component
{
    PropertyHolderComponent() {}

    z0 paint (Graphics&) override {}

    z0 updateLayout (i32 width)
    {
        auto y = 0;

        for (auto* section : sections)
        {
            section->setBounds (0, y, width, section->getPreferredHeight());
            y = section->getBottom();
        }

        setSize (width, y);
        repaint();
    }

    z0 refreshAll() const
    {
        for (auto* section : sections)
            section->refreshAll();
    }

    z0 insertSection (i32 indexToInsertAt, SectionComponent* newSection)
    {
        sections.insert (indexToInsertAt, newSection);
        addAndMakeVisible (newSection, 0);
    }

    SectionComponent* getSectionWithNonEmptyName (i32 targetIndex) const noexcept
    {
        auto index = 0;
        for (auto* section : sections)
        {
            if (section->getName().isNotEmpty())
                if (index++ == targetIndex)
                    return section;
        }

        return nullptr;
    }

    OwnedArray<SectionComponent> sections;

    DRX_DECLARE_NON_COPYABLE (PropertyHolderComponent)
};


//==============================================================================
PropertyPanel::PropertyPanel()
{
    init();
}

PropertyPanel::PropertyPanel (const Txt& name)  : Component (name)
{
    init();
}

z0 PropertyPanel::init()
{
    messageWhenEmpty = TRANS ("(nothing selected)");

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (propertyHolderComponent = new PropertyHolderComponent());
    viewport.setFocusContainerType (FocusContainerType::keyboardFocusContainer);
}

PropertyPanel::~PropertyPanel()
{
    clear();
}

//==============================================================================
z0 PropertyPanel::paint (Graphics& g)
{
    if (isEmpty())
    {
        g.setColor (Colors::black.withAlpha (0.5f));
        g.setFont (14.0f);
        g.drawText (messageWhenEmpty, getLocalBounds().withHeight (30),
                    Justification::centred, true);
    }
}

z0 PropertyPanel::resized()
{
    viewport.setBounds (getLocalBounds());
    updatePropHolderLayout();
}

//==============================================================================
z0 PropertyPanel::clear()
{
    if (! isEmpty())
    {
        propertyHolderComponent->sections.clear();
        updatePropHolderLayout();
    }
}

b8 PropertyPanel::isEmpty() const
{
    return propertyHolderComponent->sections.size() == 0;
}

i32 PropertyPanel::getTotalContentHeight() const
{
    return propertyHolderComponent->getHeight();
}

z0 PropertyPanel::addProperties (const Array<PropertyComponent*>& newProperties,
                                   i32 extraPaddingBetweenComponents)
{
    if (isEmpty())
        repaint();

    propertyHolderComponent->insertSection (-1, new SectionComponent ({}, newProperties, true, extraPaddingBetweenComponents));
    updatePropHolderLayout();
}

z0 PropertyPanel::addSection (const Txt& sectionTitle,
                                const Array<PropertyComponent*>& newProperties,
                                b8 shouldBeOpen,
                                i32 indexToInsertAt,
                                i32 extraPaddingBetweenComponents)
{
    jassert (sectionTitle.isNotEmpty());

    if (isEmpty())
        repaint();

    propertyHolderComponent->insertSection (indexToInsertAt, new SectionComponent (sectionTitle,
                                                                                   newProperties,
                                                                                   shouldBeOpen,
                                                                                   extraPaddingBetweenComponents));

    updatePropHolderLayout();
}

z0 PropertyPanel::updatePropHolderLayout() const
{
    auto maxWidth = viewport.getMaximumVisibleWidth();
    propertyHolderComponent->updateLayout (maxWidth);

    auto newMaxWidth = viewport.getMaximumVisibleWidth();
    if (maxWidth != newMaxWidth)
    {
        // need to do this twice because of scrollbars changing the size, etc.
        propertyHolderComponent->updateLayout (newMaxWidth);
    }
}

z0 PropertyPanel::refreshAll() const
{
    propertyHolderComponent->refreshAll();
}

//==============================================================================
StringArray PropertyPanel::getSectionNames() const
{
    StringArray s;

    for (auto* section : propertyHolderComponent->sections)
    {
        if (section->getName().isNotEmpty())
            s.add (section->getName());
    }

    return s;
}

b8 PropertyPanel::isSectionOpen (i32 sectionIndex) const
{
    if (auto* s = propertyHolderComponent->getSectionWithNonEmptyName (sectionIndex))
        return s->isOpen;

    return false;
}

z0 PropertyPanel::setSectionOpen (i32 sectionIndex, b8 shouldBeOpen)
{
    if (auto* s = propertyHolderComponent->getSectionWithNonEmptyName (sectionIndex))
        s->setOpen (shouldBeOpen);
}

z0 PropertyPanel::setSectionEnabled (i32 sectionIndex, b8 shouldBeEnabled)
{
    if (auto* s = propertyHolderComponent->getSectionWithNonEmptyName (sectionIndex))
        s->setEnabled (shouldBeEnabled);
}

z0 PropertyPanel::removeSection (i32 sectionIndex)
{
    if (auto* s = propertyHolderComponent->getSectionWithNonEmptyName (sectionIndex))
    {
        propertyHolderComponent->sections.removeObject (s);
        updatePropHolderLayout();
    }
}

//==============================================================================
std::unique_ptr<XmlElement> PropertyPanel::getOpennessState() const
{
    auto xml = std::make_unique<XmlElement> ("PROPERTYPANELSTATE");

    xml->setAttribute ("scrollPos", viewport.getViewPositionY());

    auto sections = getSectionNames();
    for (auto s : sections)
    {
        if (s.isNotEmpty())
        {
            auto* e = xml->createNewChildElement ("SECTION");
            e->setAttribute ("name", s);
            e->setAttribute ("open", isSectionOpen (sections.indexOf (s)) ? 1 : 0);
        }
    }

    return xml;
}

z0 PropertyPanel::restoreOpennessState (const XmlElement& xml)
{
    if (xml.hasTagName ("PROPERTYPANELSTATE"))
    {
        auto sections = getSectionNames();

        for (auto* e : xml.getChildWithTagNameIterator ("SECTION"))
        {
            setSectionOpen (sections.indexOf (e->getStringAttribute ("name")),
                            e->getBoolAttribute ("open"));
        }

        viewport.setViewPosition (viewport.getViewPositionX(),
                                  xml.getIntAttribute ("scrollPos", viewport.getViewPositionY()));
    }
}

//==============================================================================
z0 PropertyPanel::setMessageWhenEmpty (const Txt& newMessage)
{
    if (messageWhenEmpty != newMessage)
    {
        messageWhenEmpty = newMessage;
        repaint();
    }
}

const Txt& PropertyPanel::getMessageWhenEmpty() const noexcept
{
    return messageWhenEmpty;
}

} // namespace drx
