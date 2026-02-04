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
    A factory object which can create ToolbarItemComponent objects.

    A subclass of ToolbarItemFactory publishes a set of types of toolbar item
    that it can create.

    Each type of item is identified by a unique ID, and multiple instances of an
    item type can exist at once (even on the same toolbar, e.g. spacers or separator
    bars).

    @see Toolbar, ToolbarItemComponent, ToolbarButton

    @tags{GUI}
*/
class DRX_API  ToolbarItemFactory
{
public:
    //==============================================================================
    ToolbarItemFactory();

    /** Destructor. */
    virtual ~ToolbarItemFactory();

    //==============================================================================
    /** A set of reserved item ID values, used for the built-in item types.
    */
    enum SpecialItemIds
    {
        separatorBarId      = -1,   /**< The item ID for a vertical (or horizontal) separator bar that
                                         can be placed between sets of items to break them into groups. */
        spacerId            = -2,   /**< The item ID for a fixed-width space that can be placed between
                                         items.*/
        flexibleSpacerId    = -3    /**< The item ID for a gap that pushes outwards against the things on
                                         either side of it, filling any available space. */
    };

    //==============================================================================
    /** Must return a list of the IDs for all the item types that this factory can create.

        The ids should be added to the array that is passed-in.

        An item ID can be any integer you choose, except for 0, which is considered a null ID,
        and the predefined IDs in the SpecialItemIds enum.

        You should also add the built-in types (separatorBarId, spacerId and flexibleSpacerId)
        to this list if you want your toolbar to be able to contain those items.

        The list returned here is used by the ToolbarItemPalette class to obtain its list
        of available items, and their order on the palette will reflect the order in which
        they appear on this list.

        @see ToolbarItemPalette
    */
    virtual z0 getAllToolbarItemIds (Array <i32>& ids) = 0;

    /** Must return the set of items that should be added to a toolbar as its default set.

        This method is used by Toolbar::addDefaultItems() to determine which items to
        create.

        The items that your method adds to the array that is passed-in will be added to the
        toolbar in the same order. Items can appear in the list more than once.
    */
    virtual z0 getDefaultItemSet (Array <i32>& ids) = 0;

    /** Must create an instance of one of the items that the factory lists in its
        getAllToolbarItemIds() method.

        The itemId parameter can be any of the values listed by your getAllToolbarItemIds()
        method, except for the built-in item types from the SpecialItemIds enum, which
        are created internally by the toolbar code.

        Try not to keep a pointer to the object that is returned, as it will be deleted
        automatically by the toolbar, and remember that multiple instances of the same
        item type are likely to exist at the same time.
    */
    virtual ToolbarItemComponent* createItem (i32 itemId) = 0;
};

} // namespace drx
