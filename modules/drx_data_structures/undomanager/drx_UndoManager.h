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
    Manages a list of undo/redo commands.

    An UndoManager object keeps a list of past actions and can use these actions
    to move backwards and forwards through an undo history.

    To use it, create subclasses of UndoableAction which perform all the
    actions you need, then when you need to actually perform an action, create one
    and pass it to the UndoManager's perform() method.

    The manager also uses the concept of 'transactions' to group the actions
    together - all actions performed between calls to beginNewTransaction() are
    grouped together and are all undone/redone as a group.

    The UndoManager is a ChangeBroadcaster, so listeners can register to be told
    when actions are performed or undone.

    @see UndoableAction

    @tags{DataStructures}
*/
class DRX_API  UndoManager  : public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates an UndoManager.

        @param maxNumberOfUnitsToKeep       each UndoableAction object returns a value
                                            to indicate how much storage it takes up
                                            (UndoableAction::getSizeInUnits()), so this
                                            lets you specify the maximum total number of
                                            units that the undomanager is allowed to
                                            keep in memory before letting the older actions
                                            drop off the end of the list.
        @param minimumTransactionsToKeep    this specifies the minimum number of transactions
                                            that will be kept, even if this involves exceeding
                                            the amount of space specified in maxNumberOfUnitsToKeep
    */
    UndoManager (i32 maxNumberOfUnitsToKeep = 30000,
                 i32 minimumTransactionsToKeep = 30);

    /** Destructor. */
    ~UndoManager() override;

    //==============================================================================
    /** Deletes all stored actions in the list. */
    z0 clearUndoHistory();

    /** Returns the current amount of space to use for storing UndoableAction objects.
        @see setMaxNumberOfStoredUnits
    */
    i32 getNumberOfUnitsTakenUpByStoredCommands() const;

    /** Sets the amount of space that can be used for storing UndoableAction objects.

        @param maxNumberOfUnitsToKeep       each UndoableAction object returns a value
                                            to indicate how much storage it takes up
                                            (UndoableAction::getSizeInUnits()), so this
                                            lets you specify the maximum total number of
                                            units that the undomanager is allowed to
                                            keep in memory before letting the older actions
                                            drop off the end of the list.
        @param minimumTransactionsToKeep    this specifies the minimum number of transactions
                                            that will be kept, even if this involves exceeding
                                            the amount of space specified in maxNumberOfUnitsToKeep
        @see getNumberOfUnitsTakenUpByStoredCommands
    */
    z0 setMaxNumberOfStoredUnits (i32 maxNumberOfUnitsToKeep,
                                    i32 minimumTransactionsToKeep);

    //==============================================================================
    /** Performs an action and adds it to the undo history list.

        @param action   the action to perform - this object will be deleted by
                        the UndoManager when no longer needed
        @returns true if the command succeeds - see UndoableAction::perform
        @see beginNewTransaction
    */
    b8 perform (UndoableAction* action);

    /** Performs an action and also gives it a name.

        @param action       the action to perform - this object will be deleted by
                            the UndoManager when no longer needed
        @param actionName   if this string is non-empty, the current transaction will be
                            given this name; if it's empty, the current transaction name will
                            be left unchanged. See setCurrentTransactionName()
        @returns true if the command succeeds - see UndoableAction::perform
        @see beginNewTransaction
    */
    b8 perform (UndoableAction* action, const Txt& actionName);

    /** Starts a new group of actions that together will be treated as a single transaction.

        All actions that are passed to the perform() method between calls to this
        method are grouped together and undone/redone together by a single call to
        undo() or redo().
    */
    z0 beginNewTransaction();

    /** Starts a new group of actions that together will be treated as a single transaction.

        All actions that are passed to the perform() method between calls to this
        method are grouped together and undone/redone together by a single call to
        undo() or redo().

        @param actionName   a description of the transaction that is about to be
                            performed
    */
    z0 beginNewTransaction (const Txt& actionName);

    /** Changes the name stored for the current transaction.

        Each transaction is given a name when the beginNewTransaction() method is
        called, but this can be used to change that name without starting a new
        transaction.
    */
    z0 setCurrentTransactionName (const Txt& newName);

    /** Returns the name of the current transaction.
        @see setCurrentTransactionName
    */
    Txt getCurrentTransactionName() const;

    //==============================================================================
    /** Возвращает true, если there's at least one action in the list to undo.
        @see getUndoDescription, undo, canRedo
    */
    b8 canUndo() const;

    /** Tries to roll-back the last transaction.
        @returns    true if the transaction can be undone, and false if it fails, or
                    if there aren't any transactions to undo
        @see undoCurrentTransactionOnly
    */
    b8 undo();

    /** Tries to roll-back any actions that were added to the current transaction.

        This will perform an undo() only if there are some actions in the undo list
        that were added after the last call to beginNewTransaction().

        This is useful because it lets you call beginNewTransaction(), then
        perform an operation which may or may not actually perform some actions, and
        then call this method to get rid of any actions that might have been done
        without it rolling back the previous transaction if nothing was actually
        done.

        @returns true if any actions were undone.
    */
    b8 undoCurrentTransactionOnly();

    /** Returns the name of the transaction that will be rolled-back when undo() is called.
        @see undo, canUndo, getUndoDescriptions
    */
    Txt getUndoDescription() const;

    /** Returns the names of the sequence of transactions that will be performed if undo()
        is repeatedly called. Note that for transactions where no name was provided, the
        corresponding string will be empty.
        @see undo, canUndo, getUndoDescription
    */
    StringArray getUndoDescriptions() const;

    /** Returns the time to which the state would be restored if undo() was to be called.
        If an undo isn't currently possible, it'll return Time().
    */
    Time getTimeOfUndoTransaction() const;

    /** Returns a list of the UndoableAction objects that have been performed during the
        transaction that is currently open.

        Effectively, this is the list of actions that would be undone if undoCurrentTransactionOnly()
        were to be called now.

        The first item in the list is the earliest action performed.
    */
    z0 getActionsInCurrentTransaction (Array<const UndoableAction*>& actionsFound) const;

    /** Returns the number of UndoableAction objects that have been performed during the
        transaction that is currently open.
        @see getActionsInCurrentTransaction
    */
    i32 getNumActionsInCurrentTransaction() const;

    //==============================================================================
    /** Возвращает true, если there's at least one action in the list to redo.
        @see getRedoDescription, redo, canUndo
    */
    b8 canRedo() const;

    /** Tries to redo the last transaction that was undone.
        @returns   true if the transaction can be redone, and false if it fails, or
                   if there aren't any transactions to redo
    */
    b8 redo();

    /** Returns the name of the transaction that will be redone when redo() is called.
        @see redo, canRedo, getRedoDescriptions
    */
    Txt getRedoDescription() const;

    /** Returns the names of the sequence of transactions that will be performed if redo()
        is repeatedly called. Note that for transactions where no name was provided, the
        corresponding string will be empty.
        @see redo, canRedo, getRedoDescription
    */
    StringArray getRedoDescriptions() const;

    /** Returns the time to which the state would be restored if redo() was to be called.
        If a redo isn't currently possible, it'll return Time::getCurrentTime().
        @see redo, canRedo
    */
    Time getTimeOfRedoTransaction() const;

    /** Возвращает true, если the caller code is in the middle of an undo or redo action. */
    b8 isPerformingUndoRedo() const;

private:
    //==============================================================================
    struct ActionSet;
    OwnedArray<ActionSet> transactions, stashedFutureTransactions;
    Txt newTransactionName;
    i32 totalUnitsStored = 0, maxNumUnitsToKeep = 0, minimumTransactionsToKeep = 0, nextIndex = 0;
    b8 newTransaction = true, isInsideUndoRedoCall = false;
    ActionSet* getCurrentSet() const;
    ActionSet* getNextSet() const;
    z0 moveFutureTransactionsToStash();
    z0 restoreStashedFutureTransactions();
    z0 dropOldTransactionsIfTooLarge();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UndoManager)
};

} // namespace drx
