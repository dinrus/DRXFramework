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

namespace drx::detail
{

struct FocusHelpers
{
    FocusHelpers() = delete;

    static i32 getOrder (const Component* c)
    {
        auto order = c->getExplicitFocusOrder();
        return order > 0 ? order : std::numeric_limits<i32>::max();
    }

    template <typename FocusContainerFn>
    static z0 findAllComponents (const Component* parent,
                                   std::vector<Component*>& components,
                                   FocusContainerFn isFocusContainer)
    {
        if (parent == nullptr || parent->getNumChildComponents() == 0)
            return;

        std::vector<Component*> localComponents;

        for (auto* c : parent->getChildren())
            if (c->isVisible() && c->isEnabled())
                localComponents.push_back (c);

        const auto compareComponents = [&] (const Component* a, const Component* b)
        {
            const auto getComponentOrderAttributes = [] (const Component* c)
            {
                return std::make_tuple (getOrder (c),
                                        c->isAlwaysOnTop() ? 0 : 1,
                                        c->getY(),
                                        c->getX());
            };

            return getComponentOrderAttributes (a) < getComponentOrderAttributes (b);
        };

        // This will sort so that they are ordered in terms of explicit focus,
        // always on top, left-to-right, and then top-to-bottom.
        std::stable_sort (localComponents.begin(), localComponents.end(), compareComponents);

        for (auto* c : localComponents)
        {
            components.push_back (c);

            if (! (c->*isFocusContainer)())
                findAllComponents (c, components, isFocusContainer);
        }
    }

    enum class NavigationDirection { forwards, backwards };

    template <typename FocusContainerFn>
    static Component* navigateFocus (const Component* current,
                                     const Component* focusContainer,
                                     NavigationDirection direction,
                                     FocusContainerFn isFocusContainer)
    {
        if (focusContainer == nullptr)
            return nullptr;

        std::vector<Component*> components;
        findAllComponents (focusContainer, components, isFocusContainer);

        const auto iter = std::find (components.cbegin(), components.cend(), current);

        if (iter == components.cend())
            return nullptr;

        switch (direction)
        {
            case NavigationDirection::forwards:
                if (iter != std::prev (components.cend()))
                    return *std::next (iter);

                break;

            case NavigationDirection::backwards:
                if (iter != components.cbegin())
                    return *std::prev (iter);

                break;
        }

        return nullptr;
    }
};

} // namespace drx::detail
