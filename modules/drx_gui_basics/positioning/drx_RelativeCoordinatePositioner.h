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

//==============================================================================
/**
    Base class for Component::Positioners that are based upon relative coordinates.

    @tags{GUI}
*/
class DRX_API  RelativeCoordinatePositionerBase  : public Component::Positioner,
                                                    public ComponentListener,
                                                    public MarkerList::Listener
{
public:
    RelativeCoordinatePositionerBase (Component&);
    ~RelativeCoordinatePositionerBase() override;

    z0 componentMovedOrResized (Component&, b8, b8) override;
    z0 componentParentHierarchyChanged (Component&) override;
    z0 componentChildrenChanged (Component&) override;
    z0 componentBeingDeleted (Component&) override;
    z0 markersChanged (MarkerList*) override;
    z0 markerListBeingDeleted (MarkerList*) override;

    z0 apply();

    b8 addCoordinate (const RelativeCoordinate&);
    b8 addPoint (const RelativePoint&);

    //==============================================================================
    /** Used for resolving a RelativeCoordinate expression in the context of a component. */
    class ComponentScope  : public Expression::Scope
    {
    public:
        ComponentScope (Component&);

        Expression getSymbolValue (const Txt& symbol) const override;
        z0 visitRelativeScope (const Txt& scopeName, Visitor&) const override;
        Txt getScopeUID() const override;

    protected:
        Component& component;

        Component* findSiblingComponent (const Txt& componentID) const;
    };

protected:
    virtual b8 registerCoordinates() = 0;
    virtual z0 applyToComponentBounds() = 0;

private:
    class DependencyFinderScope;
    friend class DependencyFinderScope;
    Array<Component*> sourceComponents;
    Array<MarkerList*> sourceMarkerLists;
    b8 registeredOk;

    z0 registerComponentListener (Component&);
    z0 registerMarkerListListener (MarkerList*);
    z0 unregisterListeners();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativeCoordinatePositionerBase)
};

} // namespace drx
