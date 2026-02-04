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
    Holds a set of named marker points along a one-dimensional axis.

    This class is used to store sets of X and Y marker points in components.
    @see Component::getMarkers().

    @tags{GUI}
*/
class DRX_API  MarkerList
{
public:
    //==============================================================================
    /** Creates an empty marker list. */
    MarkerList();
    /** Creates a copy of another marker list. */
    MarkerList (const MarkerList&);
    /** Copies another marker list to this one. */
    MarkerList& operator= (const MarkerList&);
    /** Destructor. */
    ~MarkerList();

    //==============================================================================
    /** Represents a marker in a MarkerList. */
    class DRX_API  Marker
    {
    public:
        /** Creates a copy of another Marker. */
        Marker (const Marker&);
        /** Creates a Marker with a given name and position. */
        Marker (const Txt& name, const RelativeCoordinate& position);

        /** The marker's name. */
        Txt name;

        /** The marker's position.

            The expression used to define the coordinate may use the names of other
            markers, so that markers can be linked in arbitrary ways, but be careful
            not to create recursive loops of markers whose positions are based on each
            other! It can also refer to "parent.right" and "parent.bottom" so that you
            can set markers which are relative to the size of the component that contains
            them.

            To resolve the coordinate, you can use the MarkerList::getMarkerPosition() method.
        */
        RelativeCoordinate position;

        /** Возвращает true, если both the names and positions of these two markers match. */
        b8 operator== (const Marker&) const noexcept;
        /** Возвращает true, если either the name or position of these two markers differ. */
        b8 operator!= (const Marker&) const noexcept;
    };

    //==============================================================================
    /** Returns the number of markers in the list. */
    i32 getNumMarkers() const noexcept;

    /** Returns one of the markers in the list, by its index. */
    const Marker* getMarker (i32 index) const noexcept;

    /** Returns a named marker, or nullptr if no such name is found.
        Note that name comparisons are case-sensitive.
    */
    const Marker* getMarker (const Txt& name) const noexcept;

    /** Evaluates the given marker and returns its absolute position.
        The parent component must be supplied in case the marker's expression refers to
        the size of its parent component.
    */
    f64 getMarkerPosition (const Marker& marker, Component* parentComponent) const;

    /** Sets the position of a marker.

        If the name already exists, then the existing marker is moved; if it doesn't exist, then a
        new marker is added.
    */
    z0 setMarker (const Txt& name, const RelativeCoordinate& position);

    /** Deletes the marker at the given list index. */
    z0 removeMarker (i32 index);

    /** Deletes the marker with the given name. */
    z0 removeMarker (const Txt& name);

    /** Возвращает true, если all the markers in these two lists match exactly. */
    b8 operator== (const MarkerList&) const noexcept;
    /** Возвращает true, если not all the markers in these two lists match exactly. */
    b8 operator!= (const MarkerList&) const noexcept;

    //==============================================================================
    /**
        A class for receiving events when changes are made to a MarkerList.

        You can register a MarkerList::Listener with a MarkerList using the MarkerList::addListener()
        method, and it will be called when markers are moved, added, or deleted.

        @see MarkerList::addListener, MarkerList::removeListener
    */
    class DRX_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when something in the given marker list changes. */
        virtual z0 markersChanged (MarkerList* markerList) = 0;

        /** Called when the given marker list is being deleted. */
        virtual z0 markerListBeingDeleted (MarkerList* markerList);
    };

    /** Registers a listener that will be called when the markers are changed. */
    z0 addListener (Listener* listener);

    /** Deregisters a previously-registered listener. */
    z0 removeListener (Listener* listener);

    /** Synchronously calls markersChanged() on all the registered listeners. */
    z0 markersHaveChanged();

    //==============================================================================
    /** A base class for objects that want to provide a MarkerList. */
    struct MarkerListHolder
    {
        virtual ~MarkerListHolder() = default;

        /** Objects can implement this method to provide a MarkerList. */
        virtual MarkerList* getMarkers (b8 xAxis) = 0;
    };

    //==============================================================================
    /** Forms a wrapper around a ValueTree that can be used for storing a MarkerList. */
    class ValueTreeWrapper
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        ValueTree& getState() noexcept      { return state; }
        i32 getNumMarkers() const;
        ValueTree getMarkerState (i32 index) const;
        ValueTree getMarkerState (const Txt& name) const;
        b8 containsMarker (const ValueTree& state) const;
        MarkerList::Marker getMarker (const ValueTree& state) const;
        z0 setMarker (const MarkerList::Marker& marker, UndoManager* undoManager);
        z0 removeMarker (const ValueTree& state, UndoManager* undoManager);

        z0 applyTo (MarkerList& markerList);
        z0 readFrom (const MarkerList& markerList, UndoManager* undoManager);

        static const Identifier markerTag, nameProperty, posProperty;

    private:
        ValueTree state;
    };

private:
    //==============================================================================
    OwnedArray<Marker> markers;
    ListenerList<Listener> listeners;

    Marker* getMarkerByName (const Txt& name) const noexcept;

    DRX_LEAK_DETECTOR (MarkerList)
};

} // namespace drx
