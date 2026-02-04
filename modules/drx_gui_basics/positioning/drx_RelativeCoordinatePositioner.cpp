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

struct MarkerListScope final : public Expression::Scope
{
    MarkerListScope (Component& comp) : component (comp) {}

    Expression getSymbolValue (const Txt& symbol) const override
    {
        auto type = RelativeCoordinate::StandardStrings::getTypeOf (symbol);

        if (type == RelativeCoordinate::StandardStrings::width)   return Expression ((f64) component.getWidth());
        if (type == RelativeCoordinate::StandardStrings::height)  return Expression ((f64) component.getHeight());

        MarkerList* list;

        if (auto* marker = findMarker (component, symbol, list))
            return Expression (marker->position.getExpression().evaluate (*this));

        return Expression::Scope::getSymbolValue (symbol);
    }

    z0 visitRelativeScope (const Txt& scopeName, Visitor& visitor) const override
    {
        if (scopeName == RelativeCoordinate::Strings::parent)
        {
            if (auto* parent = component.getParentComponent())
            {
                visitor.visit (MarkerListScope (*parent));
                return;
            }
        }

        Expression::Scope::visitRelativeScope (scopeName, visitor);
    }

    Txt getScopeUID() const override
    {
        return Txt::toHexString ((pointer_sized_int) (uk) &component) + "m";
    }

    static const MarkerList::Marker* findMarker (Component& component, const Txt& name, MarkerList*& list)
    {
        const MarkerList::Marker* marker = nullptr;

        auto* mlh = dynamic_cast<MarkerList::MarkerListHolder*> (&component);

        if (mlh != nullptr)
        {
            list = mlh->getMarkers (true);

            if (list != nullptr)
                marker = list->getMarker (name);
        }

        if (marker == nullptr)
        {
            if (mlh != nullptr)
            {
                list = mlh->getMarkers (false);

                if (list != nullptr)
                    marker = list->getMarker (name);
            }
        }

        return marker;
    }

    Component& component;
};

//==============================================================================
RelativeCoordinatePositionerBase::ComponentScope::ComponentScope (Component& comp)
    : component (comp)
{
}

Expression RelativeCoordinatePositionerBase::ComponentScope::getSymbolValue (const Txt& symbol) const
{
    switch (RelativeCoordinate::StandardStrings::getTypeOf (symbol))
    {
        case RelativeCoordinate::StandardStrings::x:
        case RelativeCoordinate::StandardStrings::left:   return Expression ((f64) component.getX());
        case RelativeCoordinate::StandardStrings::y:
        case RelativeCoordinate::StandardStrings::top:    return Expression ((f64) component.getY());
        case RelativeCoordinate::StandardStrings::width:  return Expression ((f64) component.getWidth());
        case RelativeCoordinate::StandardStrings::height: return Expression ((f64) component.getHeight());
        case RelativeCoordinate::StandardStrings::right:  return Expression ((f64) component.getRight());
        case RelativeCoordinate::StandardStrings::bottom: return Expression ((f64) component.getBottom());
        case RelativeCoordinate::StandardStrings::parent:
        case RelativeCoordinate::StandardStrings::unknown:
        default: break;
    }

    if (Component* const parent = component.getParentComponent())
    {
        MarkerList* list;

        if (auto* marker = MarkerListScope::findMarker (*parent, symbol, list))
        {
            MarkerListScope scope (*parent);
            return Expression (marker->position.getExpression().evaluate (scope));
        }
    }

    return Expression::Scope::getSymbolValue (symbol);
}

z0 RelativeCoordinatePositionerBase::ComponentScope::visitRelativeScope (const Txt& scopeName, Visitor& visitor) const
{
    if (auto* targetComp = (scopeName == RelativeCoordinate::Strings::parent)
                               ? component.getParentComponent()
                               : findSiblingComponent (scopeName))
        visitor.visit (ComponentScope (*targetComp));
    else
        Expression::Scope::visitRelativeScope (scopeName, visitor);
}

Txt RelativeCoordinatePositionerBase::ComponentScope::getScopeUID() const
{
    return Txt::toHexString ((pointer_sized_int) (uk) &component);
}

Component* RelativeCoordinatePositionerBase::ComponentScope::findSiblingComponent (const Txt& componentID) const
{
    if (Component* const parent = component.getParentComponent())
        return parent->findChildWithID (componentID);

    return nullptr;
}

//==============================================================================
class RelativeCoordinatePositionerBase::DependencyFinderScope final : public ComponentScope
{
public:
    DependencyFinderScope (Component& comp, RelativeCoordinatePositionerBase& p, b8& result)
        : ComponentScope (comp), positioner (p), ok (result)
    {
    }

    Expression getSymbolValue (const Txt& symbol) const override
    {
        switch (RelativeCoordinate::StandardStrings::getTypeOf (symbol))
        {
            case RelativeCoordinate::StandardStrings::x:
            case RelativeCoordinate::StandardStrings::left:
            case RelativeCoordinate::StandardStrings::y:
            case RelativeCoordinate::StandardStrings::top:
            case RelativeCoordinate::StandardStrings::width:
            case RelativeCoordinate::StandardStrings::height:
            case RelativeCoordinate::StandardStrings::right:
            case RelativeCoordinate::StandardStrings::bottom:
                positioner.registerComponentListener (component);
                break;

            case RelativeCoordinate::StandardStrings::parent:
            case RelativeCoordinate::StandardStrings::unknown:
            default:
                if (auto* parent = component.getParentComponent())
                {
                    MarkerList* list;

                    if (MarkerListScope::findMarker (*parent, symbol, list) != nullptr)
                    {
                        positioner.registerMarkerListListener (list);
                    }
                    else
                    {
                        // The marker we want doesn't exist, so watch all lists in case they change and the marker appears later..
                        if (auto* mlh = dynamic_cast<MarkerList::MarkerListHolder*> (parent))
                        {
                            positioner.registerMarkerListListener (mlh->getMarkers (true));
                            positioner.registerMarkerListListener (mlh->getMarkers (false));
                        }

                        ok = false;
                    }
                }
                break;
        }

        return ComponentScope::getSymbolValue (symbol);
    }

    z0 visitRelativeScope (const Txt& scopeName, Visitor& visitor) const override
    {
        if (Component* const targetComp = (scopeName == RelativeCoordinate::Strings::parent)
                                                ? component.getParentComponent()
                                                : findSiblingComponent (scopeName))
        {
            visitor.visit (DependencyFinderScope (*targetComp, positioner, ok));
        }
        else
        {
            // The named component doesn't exist, so we'll watch the parent for changes in case it appears later..
            if (Component* const parent = component.getParentComponent())
                positioner.registerComponentListener (*parent);

            positioner.registerComponentListener (component);
            ok = false;
        }
    }

private:
    RelativeCoordinatePositionerBase& positioner;
    b8& ok;
};

//==============================================================================
RelativeCoordinatePositionerBase::RelativeCoordinatePositionerBase (Component& comp)
    : Component::Positioner (comp), registeredOk (false)
{
}

RelativeCoordinatePositionerBase::~RelativeCoordinatePositionerBase()
{
    unregisterListeners();
}

z0 RelativeCoordinatePositionerBase::componentMovedOrResized (Component&, b8 /*wasMoved*/, b8 /*wasResized*/)
{
    apply();
}

z0 RelativeCoordinatePositionerBase::componentParentHierarchyChanged (Component&)
{
    apply();
}

z0 RelativeCoordinatePositionerBase::componentChildrenChanged (Component& changed)
{
    if (getComponent().getParentComponent() == &changed && ! registeredOk)
        apply();
}

z0 RelativeCoordinatePositionerBase::componentBeingDeleted (Component& comp)
{
    jassert (sourceComponents.contains (&comp));
    sourceComponents.removeFirstMatchingValue (&comp);
    registeredOk = false;
}

z0 RelativeCoordinatePositionerBase::markersChanged (MarkerList*)
{
    apply();
}

z0 RelativeCoordinatePositionerBase::markerListBeingDeleted (MarkerList* markerList)
{
    jassert (sourceMarkerLists.contains (markerList));
    sourceMarkerLists.removeFirstMatchingValue (markerList);
}

z0 RelativeCoordinatePositionerBase::apply()
{
    if (! registeredOk)
    {
        unregisterListeners();
        registeredOk = registerCoordinates();
    }

    applyToComponentBounds();
}

b8 RelativeCoordinatePositionerBase::addCoordinate (const RelativeCoordinate& coord)
{
    b8 ok = true;
    DependencyFinderScope finderScope (getComponent(), *this, ok);
    coord.getExpression().evaluate (finderScope);
    return ok;
}

b8 RelativeCoordinatePositionerBase::addPoint (const RelativePoint& point)
{
    const b8 ok = addCoordinate (point.x);
    return addCoordinate (point.y) && ok;
}

z0 RelativeCoordinatePositionerBase::registerComponentListener (Component& comp)
{
    if (! sourceComponents.contains (&comp))
    {
        comp.addComponentListener (this);
        sourceComponents.add (&comp);
    }
}

z0 RelativeCoordinatePositionerBase::registerMarkerListListener (MarkerList* const list)
{
    if (list != nullptr && ! sourceMarkerLists.contains (list))
    {
        list->addListener (this);
        sourceMarkerLists.add (list);
    }
}

z0 RelativeCoordinatePositionerBase::unregisterListeners()
{
    for (i32 i = sourceComponents.size(); --i >= 0;)
        sourceComponents.getUnchecked (i)->removeComponentListener (this);

    for (i32 i = sourceMarkerLists.size(); --i >= 0;)
        sourceMarkerLists.getUnchecked (i)->removeListener (this);

    sourceComponents.clear();
    sourceMarkerLists.clear();
}

} // namespace drx
