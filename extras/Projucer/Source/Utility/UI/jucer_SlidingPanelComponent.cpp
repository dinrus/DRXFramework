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
#include "jucer_SlidingPanelComponent.h"

//==============================================================================
struct SlidingPanelComponent::DotButton final : public Button
{
    DotButton (SlidingPanelComponent& sp, i32 pageIndex)
        : Button (Txt()), owner (sp), index (pageIndex) {}

    z0 paintButton (Graphics& g, b8 /*isMouseOverButton*/, b8 /*isButtonDown*/) override
    {
        g.setColor (findColor (defaultButtonBackgroundColorId));
        const auto r = getLocalBounds().reduced (getWidth() / 4).toFloat();

        if (index == owner.getCurrentTabIndex())
            g.fillEllipse (r);
        else
            g.drawEllipse (r, 1.0f);
    }

    z0 clicked() override
    {
        owner.goToTab (index);
    }

    using Button::clicked;

    SlidingPanelComponent& owner;
    i32 index;
};

//==============================================================================
SlidingPanelComponent::SlidingPanelComponent()
    : currentIndex (0), dotSize (20)
{
    addAndMakeVisible (pageHolder);
}

SlidingPanelComponent::~SlidingPanelComponent()
{
}

SlidingPanelComponent::PageInfo::~PageInfo()
{
    if (shouldDelete)
        content.deleteAndZero();
}

z0 SlidingPanelComponent::addTab (const Txt& tabName,
                                    Component* const contentComponent,
                                    const b8 deleteComponentWhenNotNeeded,
                                    i32k insertIndex)
{
    PageInfo* page = new PageInfo();
    pages.insert (insertIndex, page);
    page->content = contentComponent;
    page->dotButton.reset (new DotButton (*this, pages.indexOf (page)));
    addAndMakeVisible (page->dotButton.get());
    page->name = tabName;
    page->shouldDelete = deleteComponentWhenNotNeeded;

    pageHolder.addAndMakeVisible (contentComponent);
    resized();
}

z0 SlidingPanelComponent::goToTab (i32 targetTabIndex)
{
    currentIndex = targetTabIndex;

    Desktop::getInstance().getAnimator()
        .animateComponent (&pageHolder, pageHolder.getBounds().withX (-targetTabIndex * getWidth()),
                           1.0f, 600, false, 0.0, 0.0);

    repaint();
}

z0 SlidingPanelComponent::resized()
{
    pageHolder.setBounds (-currentIndex * getWidth(), pageHolder.getPosition().y,
                          getNumTabs() * getWidth(), getHeight());

    Rectangle<i32> content (getLocalBounds());

    Rectangle<i32> dotHolder = content.removeFromBottom (20 + dotSize)
                                 .reduced ((content.getWidth() - dotSize * getNumTabs()) / 2, 10);

    for (i32 i = 0; i < getNumTabs(); ++i)
        pages.getUnchecked (i)->dotButton->setBounds (dotHolder.removeFromLeft (dotSize));

    for (i32 i = pages.size(); --i >= 0;)
        if (Component* c = pages.getUnchecked (i)->content)
            c->setBounds (content.translated (i * content.getWidth(), 0));
}
