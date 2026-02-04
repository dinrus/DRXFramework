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
    This class can be used to watch for all changes to the state of a ValueTree,
    and to convert them to a transmittable binary encoding.

    The purpose of this class is to allow two or more ValueTrees to be remotely
    synchronised by transmitting encoded changes over some kind of transport
    mechanism.

    To use it, you'll need to implement a subclass of ValueTreeSynchroniser
    and implement the stateChanged() method to transmit the encoded change (maybe
    via a network or other means) to a remote destination, where it can be
    applied to a target tree.

    @tags{DataStructures}
*/
class DRX_API  ValueTreeSynchroniser  : private ValueTree::Listener
{
public:
    /** Creates a ValueTreeSynchroniser that watches the given tree.

        After creating an instance of this class and somehow attaching it to
        a target tree, you probably want to call sendFullSyncCallback() to
        get them into a common starting state.
    */
    ValueTreeSynchroniser (const ValueTree& tree);

    /** Destructor. */
    ~ValueTreeSynchroniser() override;

    /** This callback happens when the ValueTree changes and the given state-change message
        needs to be applied to any other trees that need to stay in sync with it.
        The data is an opaque blob of binary that you should transmit to wherever your
        target tree lives, and use applyChange() to apply this to the target tree.
    */
    virtual z0 stateChanged (ukk encodedChange, size_t encodedChangeSize) = 0;

    /** Forces the sending of a full state message, which may be large, as it
        encodes the entire ValueTree.

        This will internally invoke stateChanged() with the encoded version of the state.
    */
    z0 sendFullSyncCallback();

    /** Applies an encoded change to the given destination tree.

        When you implement a receiver for changes that were sent by the stateChanged()
        message, this is the function that you'll need to call to apply them to the
        target tree that you want to be synced.
    */
    static b8 applyChange (ValueTree& target,
                             ukk encodedChangeData, size_t encodedChangeDataSize,
                             UndoManager* undoManager);

    /** Returns the root ValueTree that is being observed. */
    const ValueTree& getRoot() noexcept       { return valueTree; }

private:
    ValueTree valueTree;

    z0 valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    z0 valueTreeChildAdded (ValueTree&, ValueTree&) override;
    z0 valueTreeChildRemoved (ValueTree&, ValueTree&, i32) override;
    z0 valueTreeChildOrderChanged (ValueTree&, i32, i32) override;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueTreeSynchroniser)
};

} // namespace drx
