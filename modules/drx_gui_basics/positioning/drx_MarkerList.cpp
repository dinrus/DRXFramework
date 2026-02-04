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

MarkerList::MarkerList()
{
}

MarkerList::MarkerList (const MarkerList& other)
{
    operator= (other);
}

MarkerList& MarkerList::operator= (const MarkerList& other)
{
    if (other != *this)
    {
        markers.clear();
        markers.addCopiesOf (other.markers);
        markersHaveChanged();
    }

    return *this;
}

MarkerList::~MarkerList()
{
    listeners.call ([this] (Listener& l) { l.markerListBeingDeleted (this); });
}

b8 MarkerList::operator== (const MarkerList& other) const noexcept
{
    if (other.markers.size() != markers.size())
        return false;

    for (i32 i = markers.size(); --i >= 0;)
    {
        const Marker* const m1 = markers.getUnchecked (i);
        jassert (m1 != nullptr);

        const Marker* const m2 = other.getMarker (m1->name);

        if (m2 == nullptr || *m1 != *m2)
            return false;
    }

    return true;
}

b8 MarkerList::operator!= (const MarkerList& other) const noexcept
{
    return ! operator== (other);
}

//==============================================================================
i32 MarkerList::getNumMarkers() const noexcept
{
    return markers.size();
}

const MarkerList::Marker* MarkerList::getMarker (i32k index) const noexcept
{
    return markers [index];
}

const MarkerList::Marker* MarkerList::getMarker (const Txt& name) const noexcept
{
    return getMarkerByName (name);
}

MarkerList::Marker* MarkerList::getMarkerByName (const Txt& name) const noexcept
{
    for (i32 i = 0; i < markers.size(); ++i)
    {
        Marker* const m = markers.getUnchecked (i);

        if (m->name == name)
            return m;
    }

    return nullptr;
}

z0 MarkerList::setMarker (const Txt& name, const RelativeCoordinate& position)
{
    if (Marker* const m = getMarkerByName (name))
    {
        if (m->position != position)
        {
            m->position = position;
            markersHaveChanged();
        }

        return;
    }

    markers.add (new Marker (name, position));
    markersHaveChanged();
}

z0 MarkerList::removeMarker (i32k index)
{
    if (isPositiveAndBelow (index, markers.size()))
    {
        markers.remove (index);
        markersHaveChanged();
    }
}

z0 MarkerList::removeMarker (const Txt& name)
{
    for (i32 i = 0; i < markers.size(); ++i)
    {
        const Marker* const m = markers.getUnchecked (i);

        if (m->name == name)
        {
            markers.remove (i);
            markersHaveChanged();
        }
    }
}

z0 MarkerList::markersHaveChanged()
{
    listeners.call ([this] (Listener& l) { l.markersChanged (this); });
}

z0 MarkerList::Listener::markerListBeingDeleted (MarkerList*)
{
}

z0 MarkerList::addListener (Listener* listener)
{
    listeners.add (listener);
}

z0 MarkerList::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

//==============================================================================
MarkerList::Marker::Marker (const Marker& other)
    : name (other.name), position (other.position)
{
}

MarkerList::Marker::Marker (const Txt& name_, const RelativeCoordinate& position_)
    : name (name_), position (position_)
{
}

b8 MarkerList::Marker::operator== (const Marker& other) const noexcept
{
    return name == other.name && position == other.position;
}

b8 MarkerList::Marker::operator!= (const Marker& other) const noexcept
{
    return ! operator== (other);
}

//==============================================================================
const Identifier MarkerList::ValueTreeWrapper::markerTag ("Marker");
const Identifier MarkerList::ValueTreeWrapper::nameProperty ("name");
const Identifier MarkerList::ValueTreeWrapper::posProperty ("position");

MarkerList::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : state (state_)
{
}

i32 MarkerList::ValueTreeWrapper::getNumMarkers() const
{
    return state.getNumChildren();
}

ValueTree MarkerList::ValueTreeWrapper::getMarkerState (i32 index) const
{
    return state.getChild (index);
}

ValueTree MarkerList::ValueTreeWrapper::getMarkerState (const Txt& name) const
{
    return state.getChildWithProperty (nameProperty, name);
}

b8 MarkerList::ValueTreeWrapper::containsMarker (const ValueTree& marker) const
{
    return marker.isAChildOf (state);
}

MarkerList::Marker MarkerList::ValueTreeWrapper::getMarker (const ValueTree& marker) const
{
    jassert (containsMarker (marker));

    return MarkerList::Marker (marker [nameProperty], RelativeCoordinate (marker [posProperty].toString()));
}

z0 MarkerList::ValueTreeWrapper::setMarker (const MarkerList::Marker& m, UndoManager* undoManager)
{
    ValueTree marker (state.getChildWithProperty (nameProperty, m.name));

    if (marker.isValid())
    {
        marker.setProperty (posProperty, m.position.toString(), undoManager);
    }
    else
    {
        marker = ValueTree (markerTag);
        marker.setProperty (nameProperty, m.name, nullptr);
        marker.setProperty (posProperty, m.position.toString(), nullptr);
        state.appendChild (marker, undoManager);
    }
}

z0 MarkerList::ValueTreeWrapper::removeMarker (const ValueTree& marker, UndoManager* undoManager)
{
    state.removeChild (marker, undoManager);
}

f64 MarkerList::getMarkerPosition (const Marker& marker, Component* parentComponent) const
{
    if (parentComponent == nullptr)
        return marker.position.resolve (nullptr);

    RelativeCoordinatePositionerBase::ComponentScope scope (*parentComponent);
    return marker.position.resolve (&scope);
}

//==============================================================================
z0 MarkerList::ValueTreeWrapper::applyTo (MarkerList& markerList)
{
    i32k numMarkers = getNumMarkers();

    StringArray updatedMarkers;

    for (i32 i = 0; i < numMarkers; ++i)
    {
        const ValueTree marker (state.getChild (i));
        const Txt name (marker [nameProperty].toString());
        markerList.setMarker (name, RelativeCoordinate (marker [posProperty].toString()));
        updatedMarkers.add (name);
    }

    for (i32 i = markerList.getNumMarkers(); --i >= 0;)
        if (! updatedMarkers.contains (markerList.getMarker (i)->name))
            markerList.removeMarker (i);
}

z0 MarkerList::ValueTreeWrapper::readFrom (const MarkerList& markerList, UndoManager* undoManager)
{
    state.removeAllChildren (undoManager);

    for (i32 i = 0; i < markerList.getNumMarkers(); ++i)
        setMarker (*markerList.getMarker (i), undoManager);
}

} // namespace drx
